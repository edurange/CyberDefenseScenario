
INSTALLDIR=/usr/sbin/uptimechecker

all: install

install:
	mkdir $(INSTALLDIR)
	cp Server.pm $(INSTALLDIR)/
	cp uptime.config $(INSTALLDIR)/
	cp uptime.pl $(INSTALLDIR)/
	cp uptime $(INSTALLDIR)/

uninstall:
	rm -rf $(INSTALLDIR)/

