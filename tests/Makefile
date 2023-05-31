all:
	rm -f ../granula.cpp
	cd ../&&ln -s granula.ino granula.cpp
	clang++ -std=c++11  -g -D_GRANULA_TESTS_ -Wno-c++11-compat-deprecated-writable-strings -I./ granula_tests.cpp granula_tests_stubs.cpp ../display.cpp ../granula.cpp ../gsynth.cpp ../midi.cpp ../tone_generator.cpp ../controls.cpp ../page_adsr.cpp ../page_home.cpp ../page_menu.cpp ../page_rec.cpp  ../page_splash.cpp -o granula_tests
	rm ../granula.cpp