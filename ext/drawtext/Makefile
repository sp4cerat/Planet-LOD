fonts = fonts/linux-libertine.ttf fonts/klingon.ttf fonts/sazanami-mincho.ttf

.PHONY: all
all: $(fonts)
	$(MAKE) -C simple
	$(MAKE) -C unicode
	$(MAKE) -C nofreetype

fonts/linux-libertine.ttf:
	mkdir -p fonts
	wget http://downloads.sourceforge.net/project/linuxlibertine/linuxlibertine/5.1.3-2/LinLibertineTTF_5.1.3_2011_06_21.tgz
	mkdir -p linlibertine
	cd linlibertine; tar xzvf ../LinLibertineTTF_5.1.3_2011_06_21.tgz
	rm -f LinLibertineTTF_5.1.3_2011_06_21.tgz
	cp linlibertine/LinLibertine_R.ttf $@
	rm -rf linlibertine

fonts/sazanami-mincho.ttf:
	mkdir -p fonts
	wget -O sazanami-20040629.tar.bz2 'http://sourceforge.jp/frs/redir.php?m=iij&f=/efont/10087/sazanami-20040629.tar.bz2'
	tar xjvf sazanami-20040629.tar.bz2
	rm -f sazanami-20040629.tar.bz2
	cp sazanami-20040629/sazanami-mincho.ttf $@
	rm -rf sazanami-20040629

fonts/klingon.ttf:
	mkdir -p fonts
	wget http://blogs.msdn.com/cfs-file.ashx/__key/communityserver-components-postattachments/00-00-53-45-33/tlh_2D00_pIqaD_2D00_US.zip
	unzip -o tlh_2D00_pIqaD_2D00_US.zip
	rm -f tlh_2D00_pIqaD_2D00_US.zip
	cp tlh-pIqaD-US/pIqaD.ttf $@
	rm -rf tlh-pIqaD-US
