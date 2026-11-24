# Changelog

## [0.14.1](https://github.com/cvilas/grape/compare/v0.14.0...v0.14.1) (2026-11-24)


### Bug Fixes

* **base:** cleans up shared memory on failure ([437baf0](https://github.com/cvilas/grape/commit/437baf0884d370a5826b65bebb708d01ab908295))
* **ipc:** makes publisher move and thread safe ([687c939](https://github.com/cvilas/grape/commit/687c939b3ab56d0926599dd4507d3d203879cd60))
* **linalg:** simplifies code with deducing-this ([cf9a23e](https://github.com/cvilas/grape/commit/cf9a23ef9ca17243be40ee9c98f6d56f2fc46921))
* **log:** make periodic flushing interruptible ([98cae99](https://github.com/cvilas/grape/commit/98cae99aaef280541bd3ced81bf8abe696c64c0f))
* **realtime:** destroys pthread mutex attributes when done with it ([cc9c0ef](https://github.com/cvilas/grape/commit/cc9c0ef937d15e1d297b3d0e7262f150e9bd1183))
* **realtime:** reports mallopt failures accurately ([095ed20](https://github.com/cvilas/grape/commit/095ed20486824c653f9e202899f9c94714e7944e))

## [0.14.0](https://github.com/cvilas/grape/compare/v0.13.0...v0.14.0) (2026-11-22)

* **build:** sets baseline for CPU instruction sets ([6ff86d0](https://github.com/cvilas/grape/commit/6ff86d019ff3c5e9c035578d281b6531a3c61b80))
* **external:** pass arch setting to dependencies ([8e22c1f](https://github.com/cvilas/grape/commit/8e22c1f19fef4caeaaf820d9aca46b8e1ebdf1de))
* **linalg:** introduces linear algebra library ([a2895f7](https://github.com/cvilas/grape/commit/a2895f79afb4d331db1e96b71668451eb11243ee))
* **utils:** adds a macro for inlining functions ([29bfdab](https://github.com/cvilas/grape/commit/29bfdabbe56437f33e82a0f29cb65c16ec16fa28))
* **external:** patches SDL3 source_group prefix check ([9a7706d](https://github.com/cvilas/grape/commit/9a7706d68db07ad52f6abe7d95ba5d5563ccae46))
* **external:** works around a bug in SDL3 cmake scripts ([608919c](https://github.com/cvilas/grape/commit/608919c9ec18ff23005303b6f92bb843756d088b))
* **realtime:** adds seqlock memory fences to spmcq ([8856b16](https://github.com/cvilas/grape/commit/8856b1664f25c99773939b8d3a2b6c41f27ebb9d))
* **splat:** depends on linalg for maths ([4df5f72](https://github.com/cvilas/grape/commit/4df5f722ec6ae6288d9963406da3d1a66fa0d4c5))

## [0.13.0](https://github.com/cvilas/grape/compare/v0.12.0...v0.13.0) (2026-11-04)

* **external:** updates SDL to v3.4.20 ([06ea9e1](https://github.com/cvilas/grape/commit/06ea9e16c1ff7a1606d879edfaf3343531271b76))
* **build:** pins uv wheel build to the CMake-detected Python interpreter ([f2ec597](https://github.com/cvilas/grape/commit/f2ec5979f78ce5b4a5f5bebbaec645cb1f50b0e1))

## [0.12.0](https://github.com/cvilas/grape/compare/v0.11.0...v0.12.0) (2026-10-25)

* **gui:** packages imgui and implot separately ([4ecef5d](https://github.com/cvilas/grape/commit/4ecef5d0f3573e266b5188001e0e66f5f1acd177))

## [0.11.0](https://github.com/cvilas/grape/compare/v0.10.0...v0.11.0) (2026-10-17)

* **external:** updates catch2 to v3.16.1 ([a38757b](https://github.com/cvilas/grape/commit/a38757be81539993790b0ebacbd268a449069349))
* **splat:** introduces experimental gaussian splats viewer ([b351c1c](https://github.com/cvilas/grape/commit/b351c1c1925faba9e51191f1b01842ea092765ec))
* **docs:** fixes errors in install instructions ([4cb8731](https://github.com/cvilas/grape/commit/4cb873151f3301861475cab49109735f494f3335))
* **many:** supports CTAD properly ([4bc4472](https://github.com/cvilas/grape/commit/4bc4472804b29b825e9fe1ddea88ef25e159e7ea))

## [0.10.0](https://github.com/cvilas/grape/compare/v0.9.0...v0.2.0) (2026-10-03)

* **external:** updates SDL to v3.4.18 ([5442711](https://github.com/cvilas/grape/commit/5442711be4b75bb374bb29533098eedf1d811199))
* **toolchain:** upgrades llvm to v23 ([e45eed3](https://github.com/cvilas/grape/commit/e45eed35d225c08676e937b0da874371e4a4a73f))
* **toolchain:** updates minimum supported cmake version ([ff651ed](https://github.com/cvilas/grape/commit/ff651ed8fc27b9f662eb89a19a2b1cdc6b3ebf8b))
* **ci:** linter validates formatting of every commit message, not just PR title ([b5b7b18](https://github.com/cvilas/grape/commit/b5b7b18c3f9e5bf229276fcf76253545056b1214))
* **external:** Updates Catch2 ([79db5a6](https://github.com/cvilas/grape/commit/79db5a62e8c62abe30f017d470f556a70f6db5d8))
* **external:** Updates nanobind ([ecd24f4](https://github.com/cvilas/grape/commit/ecd24f4e1e1c318a6237441e83631d1eca9098af))
* **all:** Annotate a few branches with [[unlikely]] ([f6d02c4](https://github.com/cvilas/grape/commit/f6d02c4601378ca8b793f5c65ab449d04fd9d5eb))
* **ci:** Updates cache on merge to main branch ([3aad941](https://github.com/cvilas/grape/commit/3aad941ade044ac7eca3bd0e824b8e06dcc722c2))
* **external:** Updates nanobind ([156e043](https://github.com/cvilas/grape/commit/156e043894f68d7f66a38629c723e89788761623))
* **build:** Simplifies nanobind integration ([7c6ca55](https://github.com/cvilas/grape/commit/7c6ca55f2ead72a259b59dd40c71f359a6e08aeb))
* **joystick:** Passes a POD type as a copy not a reference ([dba5b95](https://github.com/cvilas/grape/commit/dba5b95a8fd11e70a89b2d831992b84b7fdca2c1))
* **probe:** Simplifies an accumulation loop ([b058942](https://github.com/cvilas/grape/commit/b058942fa5da826cf741f29e5e0c6f1715dd2961))
* **serdes:** Improves benchmarking ([cbae3aa](https://github.com/cvilas/grape/commit/cbae3aa1452bf8a5ced7a9cbfe6ae16e3b87b792))
* **base:** Disallows string construction from nullptr ([495b9c3](https://github.com/cvilas/grape/commit/495b9c31ff1fd96fb702cd4f6851b77248ff787b))
* **external:** Updates nanobind and ftxui ([51165e9](https://github.com/cvilas/grape/commit/51165e9f5d8681114c4d69ba87b359c0b3df4510))
* **ci:** move release-please metadata out of workflows ([15166fb](https://github.com/cvilas/grape/commit/15166fbc1ec9a8fe563cc6ee1d71ce7a17315166))
* **serdes:** Renames a concept to clarify its scope ([02f0cb0](https://github.com/cvilas/grape/commit/02f0cb0286fb4b8eed202a94172d4c9eb0f8c05b))
* **sereds:** Introduces automatic serialisation and deserialisation ([f89b503](https://github.com/cvilas/grape/commit/f89b5037cc9562a731400358f4c10a7016e12701))
* **external:** Updates ftxui ([8f1e0fe](https://github.com/cvilas/grape/commit/8f1e0fe1040fb7b23de2e7e23d2f15dedc8221e8))
* **external:** Updates SDL ([992bbfe](https://github.com/cvilas/grape/commit/992bbfe8f6492d9b95b355689e60c981e078771a))
* **build:** Adds to flag to programmatically indicate independent modules ([263a2a2](https://github.com/cvilas/grape/commit/263a2a27a6d64de10f9d0a34b6cfd3fc9ca9bfee))
* **ipc:** Creates serialiser once in publisher ([1e3cf2a](https://github.com/cvilas/grape/commit/1e3cf2a7abaf99203718f7beb6c2d0919955581d))
* **serdes:** Removes manual serdes across the board ([6325ab4](https://github.com/cvilas/grape/commit/6325ab42dcf1fe4685c6d355125a9f2bf47d057c))

## [0.2.0](https://github.com/cvilas/grape/compare/v0.1.19...v0.2.0) (2026-08-01)

* **build:** Introduces sematic versioning ([ab43f6a](https://github.com/cvilas/grape/commit/ab43f6a6a79d7405a954f9fcaa7273e9df9fa4d3))
* **external:** Updates Catch2 ([e8310f8](https://github.com/cvilas/grape/commit/e8310f842503ccf2af1f5293f84242eb06c81a7e))
