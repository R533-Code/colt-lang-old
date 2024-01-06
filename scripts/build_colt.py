# Builds the Colt executable using CMake.

import os

print("-- Current working directory: ", os.getcwd())
if (os.getcwd().endswith("scripts")) or os.getcwd().endswith("scripts/"):
	os.chdir("..")
	print("-- Changed working directory to the parent directory")

from generate_project import generate_project

if (generate_project() == 0):
	if (os.system("cmake --build build") == 0):
		print("\n-- CMake built successfully")
		input("Press enter to continue...")
	else:
		input("-- ABORTED: Press enter to continue...")
		exit(1)
else:
	input("-- ABORTED: Press enter to continue...")
	exit(1)