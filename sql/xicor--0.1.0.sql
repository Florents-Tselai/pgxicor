\echo Use "CREATE EXTENSION xicor" to load this file. \quit

CREATE TYPE corr_problem AS
(
    n int,
    x float8[],
    y float8[]
);

CREATE TYPE corr_result AS
(
    xicor       float8
);

CREATE FUNCTION compute_corr_statistics(corr_problem) RETURNS corr_result AS
'MODULE_PATHNAME' LANGUAGE C IMMUTABLE
                             STRICT
                             PARALLEL SAFE;

CREATE FUNCTION _agg_compute_mine_score_trans(p corr_problem, x_i float8, y_i float8)
    RETURNS corr_problem AS
$$
SELECT (p.n + 1, ARRAY_APPEND(p.x, x_i), ARRAY_APPEND(p.y, y_i))::corr_problem
$$
    LANGUAGE sql
    IMMUTABLE
    PARALLEL SAFE;

CREATE FUNCTION _agg_compute_xicor_final(p corr_problem)
    RETURNS float8 AS
$$
SELECT (compute_corr_statistics(p)).xicor
$$
    LANGUAGE sql
    IMMUTABLE
    PARALLEL SAFE;

CREATE AGGREGATE xicor (float8, float8)(
    SFUNC = _agg_compute_mine_score_trans,
    STYPE = corr_problem,
    INITCOND = '(0, {}, {})',
    FINALFUNC = _agg_compute_xicor_final
    );