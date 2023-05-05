all:
	rm -f ../granula.cpp
	cd ../&&ln -s granula.ino granula.cpp
	gcc -g -D_GRANULA_TESTS_ -I./ granula_tests.cpp granula_tests_stubs.cpp ../display.cpp ../granula.cpp ../gsynth.cpp ../menu.cpp ../midi.cpp ../tone_generator.cpp ../controls.cpp -o granula_tests
	rm ../granula.cpp