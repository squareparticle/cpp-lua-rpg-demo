for _, duration in ipairs({0, -1, math.huge, -math.huge, 0/0, 1e100, 1e-100, "bad"}) do
    assert(not pcall(FadeOutWeather, duration), "Invalid fade duration was accepted")
end
FadeOutWeather(0.25)
SetTag("test_weather_fade_complete", true)
