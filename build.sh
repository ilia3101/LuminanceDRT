gcc -c -O3 ColourPath.c
gcc -c -O3 IPT.c
gcc -c -O3 Matrix.c
gcc -c -O3 Utilities/Utilities.c
gcc -c -O3 Program.c

gcc *.o -o process_data

rm *.o