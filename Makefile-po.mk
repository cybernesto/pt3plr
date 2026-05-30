CA ?= cadius
NAME := $(basename $(PROGRAM))
PO := $(NAME).po

# Unix or Windows
ifeq ($(shell echo),)
	CP = cp $1
	MV = mv
	RM = rm
else
	CP = copy $(subst /,\,$1)
	MV = ren
	RM = del
endif

REMOVES += $(PO)

.PHONY: po emu
po: $(PO)

$(PO): $(PROGRAM)
	$(call CP, apple2/template.po $@)
	$(CP) $(PROGRAM) $(NAME).system#FF2000
	$(CA) addfile $(PO) /$(NAME) $(NAME).system#FF2000
	$(CA) addfolder $(PO) /$(NAME)/PT3 PT3
	$(RM) $(NAME).system#FF2000

emu: $(PO)
	/opt/homebrew/bin/mame apple2ee -skip_gameinfo -window -nomax -rompath ~/mame/roms -sl4 mockingboard -sl7 cffa2 -hard1 $<