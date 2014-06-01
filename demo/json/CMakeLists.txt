set(SOURCE_LIB  json/autolink.h
				json/config.h
				json/features.h
				json/forwards.h
				json/json.h
				json/reader.h
				json/value.h
				json/writer.h
				json_batchallocator.h
				json_internalarray.inl
				json_internalmap.inl
				json_reader.cpp
				json_value.cpp
				json_valueiterator.inl
				json_writer.cpp
)

add_library(jsonlib STATIC ${SOURCE_LIB})
