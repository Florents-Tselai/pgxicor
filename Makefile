EXTENSION = xicor
EXTVERSION = 0.1.0

PG_CONFIG ?= pg_config

MODULE_big = $(EXTENSION)
OBJS = src/mine.o src/vasco.o src/xicor.o

DATA = sql/xicor--0.1.0.sql

all: $(DATA)

TESTS = $(wildcard test/sql/*.sql)
REGRESS = $(patsubst test/sql/%.sql,%,$(TESTS))
REGRESS_OPTS = --inputdir=test --load-extension=$(EXTENSION)

PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)

######### DIST / RELEASE #########

.PHONY: dist

dist:
	mkdir -p dist
	git archive --format zip --prefix=$(EXTENSION)-$(EXTVERSION)/ --output dist/$(EXTENSION)-$(EXTVERSION).zip main

# for Docker
PG_MAJOR ?= 17

.PHONY: docker

docker:
	docker build --pull --no-cache --build-arg PG_MAJOR=$(PG_MAJOR) -t florents/vasco:pg$(PG_MAJOR) -t florents/vasco:$(EXTVERSION)-pg$(PG_MAJOR) .

.PHONY: docker-release

docker-release:
	docker buildx build --push --pull --no-cache --platform linux/amd64,linux/arm64 --build-arg PG_MAJOR=$(PG_MAJOR) -t florents/vasco:pg$(PG_MAJOR) -t florents/vasco:$(EXTVERSION)-pg$(PG_MAJOR) .

######### DEVELOPMENT #########

PGDATA = ./pgdata
PG_CTL = pg_ctl
.PHONY: restart-db
restart-db:
	$(PG_CTL) -D $(PGDATA) restart

stop-db:
	$(PG_CTL) -D $(PGDATA) stop

start-db:
	postgres -D $(PGDATA)

dev: restart-db uninstall clean all install installcheck restart-db