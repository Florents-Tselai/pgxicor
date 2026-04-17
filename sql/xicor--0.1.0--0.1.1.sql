\echo Use "ALTER EXTENSION xicor UPDATE TO '0.1.1'" to load this file. \quit

CREATE OR REPLACE FUNCTION xicor_trans(corr_problem, float8, float8)
    RETURNS corr_problem
    AS 'MODULE_PATHNAME'
    LANGUAGE C
    IMMUTABLE STRICT PARALLEL SAFE;
