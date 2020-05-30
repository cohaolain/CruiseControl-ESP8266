# Changelog

All notable changes to this project will be documented in this file.

The format is based on
[Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to
[Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added

-   A startup screen/logo thing. While we search for a GPS signal!

## [0.2.1] - 2020-05-29

### Added

-   The **`safetyMultiplier`**, which adds 5% to the current speed.
    This will lead to earlier warnings, and just slower driving in general.
-   More debugging serial output. Now in the form of NMEA messages.

### Changed

-   Major overhaul of GPS serial data handling.
-   Removed frame which surrounds the UI on the OLED.
-   Current list of speed limits.
-   Logic for storing new GPS readings.
-   Flipped the OLED orientation.
-   Debug serial baud rate is now 1843200
    (not useful to link it to the GPS baud rate anymore).
-   Doesn't keep buzzing if GPS signal is lost while
    speeding is detected.
-   The u8g2 library class instance is now an global variable,
    instead of manually passing a reference to it in constructors.
-   Travis builds are now initially published as drafts.
-   Moving average now only considers past 3 readings (750ms), not 10.

### Fixed

-   Incorrect declaration of number of possible speed limits
    no longer results in accessing beyond the end of an array
    when changing the current limit.

## [0.1.3] - 2020-05-19

### Added

-   New speedometer component to UI.
-   Better documentation for installing dependency libraries.
-   Warning tone now stops if we don't have reliable up-to-date GPS data.
-   There is now a cap on the max pitch of the warning buzzer.

### Changed

-   GPS data is now sent to serial for debugging purposes.
-   All baud rates are now 9600.
-   U-Blox Neo-6M configuration
    -   Update rate is now 4 Hz instead of 5 Hz.
-   Low-speed GPS data is no longer sometimes discarded
-   GPS data now must have a HDOP of less than 4, instead of 50.
-   Adjusted text alignment on invalid-fix page.

### Fixed

-   U-Blox Neo-6M configuration
    -   Now provides the correct messages.
-   Invalid fix screen now shown if the speed data given is invalid.
-   GPS smoothing resulting in new values not being stored.

### Removed

-   U-Blox Neo-7 configuration, as I can't test this for now.

## [0.1.2] - 2020-04-24

### Added

-   Checks for missing/stale serial data/GPS updates
-   Better representation of the GPS status on display
-   Pinout diagram for Heltec WiFi Kit 8
-   Build status badge for README.md

### Changed

-   Button actions now occur before and after GPS serial read,
    not just before.
-   Now can yield to h/w interrupts when waiting for serial data

## [0.1.1] - 2020-04-21

### Added

-   Travis CI integration
    -   This will check that tagged project versions' firmwares compile
        succesfully, and then add the resulting firmware image as a release on
        GitHub.

## [0.1.0] - 2020-04-21

### Added

-   First version of the project source
-   Some documentation
