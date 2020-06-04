# FastNoiseC

[FastNoise](https://github.com/Auburns/FastNoise) translated to C and with renamed functions

(Original rewrite by Ghimli, further conversion by Tatjam, original code by Auburns)

There are no functional code changes, just renaming and translating the few non supported C++ instructions to C. rnd.h is required for good quality seeding, but could be disabled and rand() used instead.
