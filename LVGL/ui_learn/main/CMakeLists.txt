set(INCLUDES "." "./ui/custom" "./ui/generated" "./ui/generated/guider_customer_fonts")
file(GLOB_RECURSE SOURCE "./*.c")

idf_component_register(SRCS ${SOURCE}
    INCLUDE_DIRS ${INCLUDES}
    REQUIRES bsp)
