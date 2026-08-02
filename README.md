# Guess-the-number

1. **Enter three numbers first** The game prompts "Guess number 1", then "2", then "3", collecting all three guesses via the keypad before anything else happens.

2. **Then generates the random number and checks low/high/match** Only *after* all 3 guesses are locked in does `showResults()` call `random(MIN_NUMBER, MAX_NUMBER + 1)` to pick the secret number, then compares each of the 3 stored guesses against it.

3. **If none match, the LRA (vibration motor) turns on** If none of the three guesses equal the target exactly, `vibrateMiss()` fires the DRV2605 haptic effect for 900ms as a "you lost" signal. (Small note: it only vibrates on a *complete* miss — if even one guess matches, no vibration, just the "You matched it!" message instead.)

4. **Game restarts automatically after showing results** `acceptEntry()` calls `showResults()` and then immediately calls `startNewGame()` right after, so a fresh round begins on its own — no button press needed to continue.

5. **Only one LCD screen** There's exactly one `LiquidCrystal_I2C lcd` object, a 16x2 character display (16 columns, 2 rows). All the game's output — prompts, entries, and results — is shown on that single small screen, which is why the code splits things like the 3 guess results across two sequential screens (it can only fit 2 lines of ~16 characters at a time).
