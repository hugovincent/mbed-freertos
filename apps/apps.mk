# Webserver Task
C_SOURCE+= \
		apps/webserver/uIP_Task.c \
		apps/webserver/httpd.c \
		apps/webserver/httpd-cgi.c \
		apps/webserver/httpd-fs.c \
		apps/webserver/http-strings.c

EXTRA_CLEAN+= \
	apps/webserver/http-strings.* apps/webserver/httpd-fsdata.c

# UIP script build rules
apps/webserver/http-strings.c: util/uip_makestrings
	@echo "  [Making uIP http-strings ]"
	@./util/uip_makestrings

apps/webserver/httpd-fsdata.c: util/uip_makefsdata apps/webserver/httpd-fs/*
	@echo "  [Making uIP httpd-fs     ]"
	@./util/uip_makefsdata apps/webserver/httpd-fs/*

apps/webserver/httpd-fs.c: apps/webserver/httpd-fsdata.c
apps/webserver/httpd.c: apps/webserver/http-strings.c
