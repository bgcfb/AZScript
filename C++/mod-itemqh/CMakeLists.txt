CU_SET_PATH("CMAKE_MOD_ITEMQH_DIR" "${CMAKE_CURRENT_LIST_DIR}")

AC_ADD_SCRIPT("${CMAKE_CURRENT_LIST_DIR}/src/Gossip_OnItemQh.cpp")

AC_ADD_SCRIPT_LOADER("GossipOnItemQh" "${CMAKE_CURRENT_LIST_DIR}/src/loader_Gossip_OnItemQh.h")
