
all:
	@echo 
	@echo --------------------------------------------------------------------------------
	@echo  Building CArcBase
	@echo --------------------------------------------------------------------------------
	@cd CArcBase/Release; $(MAKE)

	@echo 
	@echo --------------------------------------------------------------------------------
	@echo  Building CArcDeinterlace
	@echo --------------------------------------------------------------------------------
	@cd CArcDeinterlace/Release; $(MAKE)

	@echo 
	@echo --------------------------------------------------------------------------------
	@echo  Building CArcDevice
	@echo --------------------------------------------------------------------------------
	@cd CArcDevice/Release; $(MAKE)

	@echo 
	@echo --------------------------------------------------------------------------------
	@echo  Building CArcDisplay
	@echo --------------------------------------------------------------------------------
	@cd CArcDisplay/Release; $(MAKE)

	@echo 
	@echo --------------------------------------------------------------------------------
	@echo  Building CArcFitsFile
	@echo --------------------------------------------------------------------------------
	@cd CArcFitsFile/Release; $(MAKE)

	@echo 
	@echo --------------------------------------------------------------------------------
	@echo  Building CArcImage
	@echo --------------------------------------------------------------------------------
	@cd CArcImage/Release; $(MAKE)

clean:
	rm -Rf Release/*.so
	cd CArcBase/Release; $(MAKE) clean
	cd CArcDeinterlace/Release; $(MAKE) clean
	cd CArcDevice/Release; $(MAKE) clean
	cd CArcDisplay/Release; $(MAKE) clean
	cd CArcFitsFile/Release; $(MAKE) clean
	cd CArcImage/Release; $(MAKE) clean
