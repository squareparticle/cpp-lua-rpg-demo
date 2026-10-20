SetTag("test_duties_started", true)
WaitForTrigger("mop_right")

SetTag("test_duties_reached_right", true)
WaitForTrigger("mop_left")

SetTag("test_duties_reached_left", true)
