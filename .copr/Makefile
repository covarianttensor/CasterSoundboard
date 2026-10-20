.PHONY: srpm

srpm:
	chmod +x ./scripts/build-srpm.sh
	./scripts/build-srpm.sh
	mkdir -p $(outdir)
	cp -f *.src.rpm $(outdir)/
