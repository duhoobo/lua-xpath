
ROCKSPEC_DIR = rockspec
ROCKSPEC_TPL = $(ROCKSPEC_DIR)/$(PACKAGE).rockspec
ROCKSPEC = $(ROCKSPEC_DIR)/$(PACKAGE)-$(VERSION)-1.rockspec


rockspec: $(ROCKSPEC_DIR)/$(PACKAGE).rockspec dist
	sed -e 's/@MD5@/'`$(MD5SUM) $(top_distdir).zip | cut -d " " -f 1`'/g' <$(ROCKSPEC_TPL) >$(ROCKSPEC)
	rm -f $(top_distdir).zip


all-local: rockspec

clean-local:
	@rm -f $(ROCKSPEC)


rockspec_dist = $(ROCKSPEC_DIR)/$(PACKAGE).rockspec.in
rockspec_distclean = $(ROCKSPEC_DIR)/$(PACKAGE).rockspec

