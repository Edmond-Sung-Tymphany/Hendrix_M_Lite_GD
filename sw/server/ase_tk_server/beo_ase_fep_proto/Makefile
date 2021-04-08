all:
	$(NANOPB)/generator-bin/protoc --nanopb_out=. beo_ase_fep.proto

doc:
	$(DOXYGEN)
	@(cd latex; make pdf;)
	@cp latex/refman.pdf beo_ase_fep_proto.pdf
	@rm -rf latex

clean:
	rm -f *.h *.c *.pdf
