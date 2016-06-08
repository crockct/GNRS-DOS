SCAFFOLD = /usr/bin/oml2-scaffold

all: config.h mfstack_popt.h mfstack_oml.h

mfstack_popt.h: mfandroidstack.rb
	$(info "Generating OML headers")
	$(SCAFFOLD) --opts $<

mfstack_oml.h: mfandroidstack.rb
	$(info "Generating OML headers")
	$(SCAFFOLD) --oml $<

clean: 
	rm -f *.o mfstack_oml.h mfstack_popt.h 
