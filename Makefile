build-exe:
	cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=$(HOME)/cpp/pkg/vcpkg/scripts/buildsystems/vcpkg.cmake
	cmake --build build

grpc-build:
	pushd cmake/build
	cmake -S ../.. -B -DCMAKE_TOOLCHAIN_FILE=$(HOME)/cpp/pkg/vcpkg/scripts/buildsystems/vcpkg.cmake
	cmake --build .
	popd