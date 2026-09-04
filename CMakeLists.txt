
add_library(lwl STATIC
	src/lwl_dlList.c
	src/lwl_heap.c
	src/lwl_mdlList.c
	src/lwl_ptrHeap.c
	src/lwl_rbTree.c
	src/lwl_slList.c
)

# If parent needs to include headers from this directory, expose them
target_include_directories(lwl PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)
