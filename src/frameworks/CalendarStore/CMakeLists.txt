project(CalendarStore)

remove_sdk_framework(CalendarStore)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "2030.2.1")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(CalendarStore
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/CalendarStore"
)

add_framework(CalendarStore
    FAT
    CURRENT_VERSION
    VERSION ${FRAMEWORK_VERSION}

    SOURCES
        src/CalendarStore.m
        src/CalPredicate.m
        src/CalEventPredicate.m
        src/CalAttendee.m
        src/CalGroup.m
        src/CalTaskPredicate.m
        src/CalCalendarStore.m
        src/CalCalendar.m
        src/CalAlarm.m
        src/CalEvent.m
        src/CalRecurrenceRule.m
        src/CalNthWeekDay.m
        src/CalRecurrenceEnd.m
        src/CalCalendarItemPredicate.m
        src/CalCalendarItem.m
        src/CalTask.m

    DEPENDENCIES
        system
        objc
        Foundation
)
