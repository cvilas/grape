# Changelog

## [0.8.0](https://github.com/cvilas/grape/compare/v0.7.0...v0.8.0) (2026-09-02)


### Features

* **build:** updates minimum supported cmake version ([ff651ed](https://github.com/cvilas/grape/commit/ff651ed8fc27b9f662eb89a19a2b1cdc6b3ebf8b))


### Bug Fixes

* **ci:** linter validates formatting of every commit message, not just PR title ([b5b7b18](https://github.com/cvilas/grape/commit/b5b7b18c3f9e5bf229276fcf76253545056b1214))

## [0.7.0](https://github.com/cvilas/grape/compare/v0.6.0...v0.7.0) (2026-08-27)


### Features

* Updates Catch2 ([79db5a6](https://github.com/cvilas/grape/commit/79db5a62e8c62abe30f017d470f556a70f6db5d8))

## [0.6.0](https://github.com/cvilas/grape/compare/v0.5.1...v0.6.0) (2026-08-25)


### Features

* Updates nanobind ([ecd24f4](https://github.com/cvilas/grape/commit/ecd24f4e1e1c318a6237441e83631d1eca9098af))


### Performance Improvements

* Annotate a few branches with [[unlikely]] ([f6d02c4](https://github.com/cvilas/grape/commit/f6d02c4601378ca8b793f5c65ab449d04fd9d5eb))

## [0.5.1](https://github.com/cvilas/grape/compare/v0.5.0...v0.5.1) (2026-08-17)


### Bug Fixes

* **ci:** Updates cache on merge to main branch ([3aad941](https://github.com/cvilas/grape/commit/3aad941ade044ac7eca3bd0e824b8e06dcc722c2))

## [0.5.0](https://github.com/cvilas/grape/compare/v0.4.2...v0.5.0) (2026-08-16)


### Features

* **external:** Updates nanobind ([156e043](https://github.com/cvilas/grape/commit/156e043894f68d7f66a38629c723e89788761623))

## [0.4.2](https://github.com/cvilas/grape/compare/v0.4.1...v0.4.2) (2026-08-14)


### Bug Fixes

* **build:** Simplifies nanobind integration ([7c6ca55](https://github.com/cvilas/grape/commit/7c6ca55f2ead72a259b59dd40c71f359a6e08aeb))
* **joystick:** Passes a POD type as a copy not a reference ([dba5b95](https://github.com/cvilas/grape/commit/dba5b95a8fd11e70a89b2d831992b84b7fdca2c1))
* **probe:** Simplifies an accumulation loop ([b058942](https://github.com/cvilas/grape/commit/b058942fa5da826cf741f29e5e0c6f1715dd2961))
* **serdes:** Improves benchmarking ([cbae3aa](https://github.com/cvilas/grape/commit/cbae3aa1452bf8a5ced7a9cbfe6ae16e3b87b792))

## [0.4.1](https://github.com/cvilas/grape/compare/v0.4.0...v0.4.1) (2026-08-11)


### Bug Fixes

* **base:** Disallows string construction from nullptr ([495b9c3](https://github.com/cvilas/grape/commit/495b9c31ff1fd96fb702cd4f6851b77248ff787b))

## [0.4.0](https://github.com/cvilas/grape/compare/v0.3.0...v0.4.0) (2026-08-09)

* **external:** Updates nanobind and ftxui ([51165e9](https://github.com/cvilas/grape/commit/51165e9f5d8681114c4d69ba87b359c0b3df4510))
* **ci:** move release-please metadata out of workflows ([15166fb](https://github.com/cvilas/grape/commit/15166fbc1ec9a8fe563cc6ee1d71ce7a17315166))
* **serdes:** Renames a concept to clarify its scope ([02f0cb0](https://github.com/cvilas/grape/commit/02f0cb0286fb4b8eed202a94172d4c9eb0f8c05b))

## [0.3.0](https://github.com/cvilas/grape/compare/v0.2.0...v0.3.0) (2026-08-04)

* Introduces automatic serialisation and deserialisation ([f89b503](https://github.com/cvilas/grape/commit/f89b5037cc9562a731400358f4c10a7016e12701))
* Updates ftxui ([8f1e0fe](https://github.com/cvilas/grape/commit/8f1e0fe1040fb7b23de2e7e23d2f15dedc8221e8))
* Updates SDL ([992bbfe](https://github.com/cvilas/grape/commit/992bbfe8f6492d9b95b355689e60c981e078771a))
* Adds to flag to programmatically indicate independent modules ([263a2a2](https://github.com/cvilas/grape/commit/263a2a27a6d64de10f9d0a34b6cfd3fc9ca9bfee))
* **ipc:** Creates serialiser once in publisher ([1e3cf2a](https://github.com/cvilas/grape/commit/1e3cf2a7abaf99203718f7beb6c2d0919955581d))
* Removes manual serdes across the board ([6325ab4](https://github.com/cvilas/grape/commit/6325ab42dcf1fe4685c6d355125a9f2bf47d057c))

## [0.2.0](https://github.com/cvilas/grape/compare/v0.1.19...v0.2.0) (2026-08-01)

* Introduces sematic versioning ([ab43f6a](https://github.com/cvilas/grape/commit/ab43f6a6a79d7405a954f9fcaa7273e9df9fa4d3))
* Updates Catch2 ([e8310f8](https://github.com/cvilas/grape/commit/e8310f842503ccf2af1f5293f84242eb06c81a7e))
