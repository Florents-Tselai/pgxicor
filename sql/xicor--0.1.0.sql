\echo Use "CREATE EXTENSION xicor" to load this file. \quit

CREATE TYPE corr_problem AS
(
    n int,
    x float8[],
    y float8[]
);

CREATE FUNCTION xicor_trans(p corr_problem, x_i float8, y_i float8)
    RETURNS corr_problem AS
$$
SELECT (p.n + 1, ARRAY_APPEND(p.x, x_i), ARRAY_APPEND(p.y, y_i))::corr_problem
$$
    LANGUAGE sql
    IMMUTABLE
    PARALLEL SAFE;


CREATE FUNCTION xicor_final(corr_problem) RETURNS float8 AS
'MODULE_PATHNAME' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE AGGREGATE xicor (float8, float8)(
    SFUNC = xicor_trans,
    STYPE = corr_problem,
    INITCOND = '(0, {}, {})',
    FINALFUNC = xicor_final
    );