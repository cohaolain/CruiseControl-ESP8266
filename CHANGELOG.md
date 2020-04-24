# Changelog

All notable changes to this project will be documented in this file.

The format is based on
[Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to
[Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added

-   Checks for missing/stale serial data/GPS updates
-   Better representation of the GPS status on display
-   Pinout diagram for Heltec WiFi Kit 8

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
