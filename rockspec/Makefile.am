
ROCKSPEC = $(PACKAGE)-$(VERSION)-1.rockspec

$(ROCKSPEC): $(PACKAGE).rockspec dist
	sed -e 's/@MD5@/'`$(MD5SUM) $(distdir).tar.gz | \
        cut -d " " -f 1`'/g' <$(PACKAGE).rockspec > $@

EXTRA_DIST = $(PACKAGE).rockspec.in
DISTCLEANFILES = $(PACKAGE).rockspec


all-local: $(ROCKSPEC)

clean-local:
	@rm -f $(ROCKSPEC)

