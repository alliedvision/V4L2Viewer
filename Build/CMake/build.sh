
if [[ ${1,,} == "release" ]]
then
	cmake -DCMAKE_BUILD_TYPE=release ../../
	echo "Release config is being set"
elif [[ ${1,,} == "debug" ]]
then
	cmake -DCMAKE_BUILD_TYPE=debug ../../
	echo "Debug config is being set"
else
	cmake -DCMAKE_BUILD_TYPE=debug ../../
	echo "Debug config is being set"
fi

cmake --build ./
