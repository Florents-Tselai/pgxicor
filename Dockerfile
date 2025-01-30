ARG PG_MAJOR=17
FROM postgres:$PG_MAJOR
ARG PG_MAJOR

COPY . /tmp/pgxicor

RUN apt-get update && \
		apt-mark hold locales && \
		apt-get install -y --no-install-recommends libpoppler-glib-dev pkg-config wget build-essential postgresql-server-dev-$PG_MAJOR && \
		cd /tmp/pgxicor && \
		make clean && \
		make install && \
		mkdir /usr/share/doc/pgxicor && \
		cp LICENSE README.md /usr/share/doc/pgxicor && \
		rm -r /tmp/pgxicor && \
		apt-get remove -y pkg-config wget build-essential postgresql-server-dev-$PG_MAJOR && \
		apt-get autoremove -y && \
		apt-mark unhold locales && \
		rm -rf /var/lib/apt/lists/*