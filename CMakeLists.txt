# PLEASE, UPDATE THIS FILE WHEN ADDING ANY NEW C SOURCES

set(COMPONENT_SRCS
	# Cross-platform stuff
	"soundMixer.cpp"
	"soundProvider.cpp"
	"soundProviderTask.cpp"
	)
set(COMPONENT_ADD_INCLUDEDIRS ".")

register_component()

#target_compile_features(${COMPONENT_TARGET} PUBLIC cxx_std_14)
#set_property(TARGET ${COMPONENT_TARGET} PROPERTY CXX_STANDARD 14)
