#include <stdint.h>

#define C 2
#define D 4
#define V0INIT 0x736f6d6570736575ULL;
#define V1INIT 0x646f72616e646f6dULL;
#define V2INIT 0x6c7967656e657261ULL;
#define V3INIT 0x7465646279746573ULL;

__attribute__((always_inline)) inline uint64_t rotl64(uint64_t x, int c) {
	c %= 64;
	if (c == 0) {
		return x;
	}
	return (x << c) | (x >> (64 - c));
}

__attribute__((always_inline)) inline uint64_t fibonacci(uint64_t val,
                                                uint64_t k0,
                                                uint64_t k1) {
	uint64_t v0 = k0 ^ V0INIT;
	uint64_t v1 = k1 ^ V1INIT;
	uint64_t v2 = k0 ^ V2INIT;
	uint64_t v3 = k1 ^ V3INIT;

	// block 1: value
	uint64_t block1 = 0;
	v3 ^= block1;
	for (int i = 0; i < C; i++) {
		v0 += v1;
		v2 += v3;
		v1 = rotl64(v1, 13);
		v3 = rotl64(v3, 16);
		v1 ^= v0;
		v3 ^= v2;
		v0 = rotl64(v0, 32);
		v2 += v1;
		v0 += v3;
		v1 = rotl64(v1, 17);
		v3 = rotl64(v3, 21);
		v1 ^= v2;
		v3 ^= v0;
		v2 = rotl64(v2, 32);
	}
	v0 ^= block1;

	// block 2: final block
	uint64_t block2 = 8ULL << 56;
	v3 ^= block2;
	for (int i = 0; i < C; i++) {
		v0 += v1;
		v2 += v3;
		v1 = rotl64(v1, 13);
		v3 = rotl64(v3, 16);
		v1 ^= v0;
		v3 ^= v2;
		v0 = rotl64(v0, 32);
		v2 += v1;
		v0 += v3;
		v1 = rotl64(v1, 17);
		v3 = rotl64(v3, 21);
		v1 ^= v2;
		v3 ^= v0;
		v2 = rotl64(v2, 32);
	}
	v0 ^= block2;

	// finalize
	v2 ^= 0xff;
	for (int i = 0; i < D; i++) {
		v0 += v1;
		v2 += v3;
		v1 = rotl64(v1, 13);
		v3 = rotl64(v3, 16);
		v1 ^= v0;
		v3 ^= v2;
		v0 = rotl64(v0, 32);
		v2 += v1;
		v0 += v3;
		v1 = rotl64(v1, 17);
		v3 = rotl64(v3, 21);
		v1 ^= v2;
		v3 ^= v0;
		v2 = rotl64(v2, 32);
	}
	return v0 ^ v1 ^ v2 ^ v3;
}

uint64_t results[300] = {
    0x40693a2e1275c928, 0xe14c4043c119e693, 0x34a37444bfaf6064,
    0xaa64c92a05dde17c, 0x83c5bd116cb3afbb, 0x5c3b899c8c714a1d,
    0x0eb0745366886f55, 0x5bb43fbe79a74ccb, 0x5b4e9012931c277c,
    0xeea0334d14590ef1, 0x37b28071bff5a43f, 0xa1bed2c18ad101e8,
    0x357b2d805542d085, 0xe22c381dbc6a58ed, 0xf50daf3750c757fd,
    0x07cd2dc01e44f8f7, 0xfe10217dfe7dcd37, 0xa15067baea44aced,
    0xc5e3ae4c6f11810a, 0x7c9b28a05e063346, 0x12f23b9ccfd39175,
    0x16b0c396c3e156c1, 0x1ffb80c0638039c5, 0x85d51668cff324b6,
    0xb74baee7518cf5c5, 0x86097ff37afd26a2, 0xfa4642d5d07c5734,
    0x38a0bff5b13e77e7, 0x0299053ac2754e65, 0xcde3f984402e160e,
    0x59058598e68fefd9, 0x42bffc6dccc6f6d8, 0x87502b60d6fc6239,
    0xb8e936065a285f41, 0x4f559dfd69f150f9, 0x8c9eb52f4c8dce36,
    0xfe493f1b06b41cae, 0xf48558bada490a79, 0xb226d1cff2828e8b,
    0x8001906435871a38, 0x75cb6df972acd574, 0x773c72034c83b8bc,
    0x0efca5490a29c041, 0x71e14dccf0c0b91c, 0xca43018df32e63e5,
    0xee99e6a3341eff69, 0x98847e6a2f1fc802, 0x169d80c91263ac8c,
    0xcfb40bea07684121, 0xe5a8145f8894a548, 0x3c18a1f1e8fc2cf9,
    0xf632f2e2ef129503, 0x3f2897bc83d991f8, 0x353b73742e45dcee,
    0x0f2239dadd5af9e8, 0x8714eb0e2be19e89, 0x918b8011a86e02eb,
    0xd1be1b85affa3818, 0xe230cc82edf2bde5, 0x322b6b5704f68db7,
    0x02edea4ac47a05cc, 0xfe08dda08fda1da3, 0xf278dda6afaf21aa,
    0x64435695f36fdb23, 0xdecaedf472dd128a, 0x8f6d654baf8723c9,
    0x1e0891965cd9710f, 0xa93615ae1c54617a, 0x26abd4927222ca65,
    0x20f3451fefe55589, 0x216a7ecc2ef8fbb3, 0x36fbc71c5a46639a,
    0x343ca8edbe63c620, 0x388b35e6e528006a, 0xd841031f00d5ecf8,
    0xd2f7546bf1849a85, 0xabed8f9c66105b52, 0xd601e2112b5257d1,
    0x0fd91de3e12f21e5, 0x1e93072aa0ba356d, 0xaf9d1b121f41ab8d,
    0xbb76c38d28602feb, 0xfd95361ad168d20f, 0x8c90f3c6e5e8b6fc,
    0x6866627181f5c450, 0xca0a01d3b3786f34, 0x9f8ce644341906f0,
    0x65bd1af116977f2a, 0x08e8dc325e1e0281, 0x690a3ff4e305bbe3,
    0x50c2b83a8ce05488, 0x615fe44f2bc4c0f1, 0x808fb55689b93d7f,
    0x5681cc738fa9e58f, 0x920be8a8d9ee2178, 0x90b69ea942836f95,
    0x91edbc0cf0450440, 0x50954faa63597bb9, 0xa323f0ef59c7f2e3,
    0x7eb6b99d2f66dbe5, 0x57ea0b99fa509398, 0xaf3d728de2a7df62,
    0xe02939c2eee1c81d, 0xb2a1c74a1ca26902, 0x9097677d65cc9524,
    0x0d53d81bd91776b7, 0xebc24b4e7c064f4e, 0xcf3450df40624cd4,
    0x8a085e1fa37a9d32, 0x6db4ef9a1d1b3ed3, 0x374cc2f19c15de41,
    0x1d5ecb1a99cde404, 0x508c592513e7b22d, 0x6ee0749ea289da00,
    0x848964bcdd475fc2, 0xd3ddf026b288b86d, 0x7b16b16287e1c01d,
    0x075ee8b196ce8a24, 0x403d0b105df3f779, 0x795dc6a512ea9cc7,
    0x6dceb55a824e2e6a, 0xc275c0106624acd0, 0x785d2efb22ae5688,
    0xf98e5a6e0f322be0, 0xe55c4eca49d6659e, 0x7e405debb0ab1cea,
    0x38f6bb47f791ec63, 0xf8458bba4c867e8b, 0x17d1cffa3c2c8689,
    0x460c3f31b172c5db, 0x39f7b6c08eaa9639, 0xeb9cfdd7cedd6290,
    0x788e917f5e5714e0, 0xa41f14d0c9597742, 0x7c84cbcca73ad2e6,
    0xc809eda41605b6e2, 0xa0681e6732bc0985, 0xb04bc4fa51271a09,
    0xf2f5e698e927188f, 0x9b76ef649076dffb, 0x9a931323f06f7308,
    0x4c18cf40fad0e3a7, 0x117e4411ed4a58d6, 0x4e4aa77d1b10a93e,
    0x188d35e6a2eeb897, 0xe74279d93b89aee3, 0x13d8831a5725df7b,
    0x04e9f38948103633, 0xe65b20dae87f45bb, 0x2cf9f962c0343da2,
    0x0e7994f216cc7dc3, 0xefd3bf5665d50078, 0x541846aac0bb2d81,
    0xc9b95b054f65655a, 0xe698b3fb75fd62d1, 0x2c7cda65a89f4652,
    0x69a336fbdf638a75, 0x97792707ff84bed9, 0x2c24aa9e198c20e2,
    0xb9e37329027e7d8d, 0x20eed9effedbb386, 0x145b44b79d77b1ac,
    0xffa4e1a113232003, 0x6dfc6e59c406f13c, 0x13696c1f62eedae5,
    0x195a14d73a067094, 0x2e64427fd8dbab08, 0xbe6c4abe516f077c,
    0x0429d4d6fb561b0e, 0x71b52f7b0f12c238, 0xbeb8965aed0d8de7,
    0x4ef89c4e919c9af1, 0xc9ee56028e95cff1, 0xc106086db4462717,
    0x8c6bd98c8b63b38f, 0xad664f2dd841d6f3, 0xd0a81769d5ea2410,
    0x041951e37fc0785a, 0x1323451a29b19adc, 0x11fb2ca563188e97,
    0xd843857382b251cd, 0xaaf824c3a2aeb8c6, 0x8f74823a9f5fd09f,
    0xa4e540b37fb07fd5, 0x76adfe0aec46f061, 0x7a63d44dfd347721,
    0xda65f0b352fb1c07, 0xefe18429edbed094, 0x15b9c8b33d07c577,
    0xe66b4d49a7a1015c, 0xf6edf99c9e0d0d5a, 0x7f7988baf67d70ce,
    0x39b9419c4938daf7, 0x24bdaef3d4a3aa16, 0xcb4ad81855878e41,
    0x111d68b9807c2d2c, 0x870067cfb1bd4d40, 0xb67b62f39b1fb8e4,
    0x310bac2e4bf17e16, 0x63294a091a7fe022, 0xe172bc90efd21fe3,
    0x32dfc2780cfe7412, 0x0674606a71cc0c5b, 0xd7dca57739b23ef1,
    0x173aec283f8cbbfd, 0xa7f64b522fde6799, 0x5a0e4c31155164dd,
    0x2c183157e52dd986, 0x3b6feb087c631899, 0x88f98fea73811edf,
    0x8f194c53328db71c, 0x6c93ca73f4907b2a, 0x301350e1373dc89e,
    0x10c6cc31cb1e1a4a, 0xf6ff54afdbbc04c1, 0xd90e5b46ffc59b31,
    0x86f73c3273bc5920, 0x8ff5216b55c6ea1e, 0xe60c50f96d99b349,
    0x7ce65dd3da0348f7, 0x0525916b8d7157c0, 0x5dd75032248a8a10,
    0xa541fc7dbde0b281, 0x82bd638311568d8c, 0x16928b50069b5abd,
    0x2fa7e32f26c0ae72, 0x0ceb4278cb8ef746, 0x4467b7a63442abb6,
    0x4597d8f47023c9a5, 0x48d3316479cd7dd4, 0xb8e107940243ac51,
    0xe53b23d1f71dc6b8, 0x729a5367586046d3, 0x2b33c47e268c36f7,
    0x23a900434ba7f5bb, 0x33a8e4c08322d9d4, 0x96147ca6f14f0a0f,
    0xbbc4f86dd155643b, 0x30dd392e37adb436, 0xfed6d06d03c532a8,
    0x2bef5b6863d3fead, 0x950cc55e5b1f21e3, 0xacc5101a3fc09513,
    0x73f3dd3242c67776, 0x507a859430841d04, 0xe3c51608ca3efb64,
    0xf92e4bcb80284e68, 0x51b5b7fa7b8afac7, 0xc6faae9614b69a9b,
    0x386fb3ef2ec74df5, 0x4dc0ed4ea557653e, 0x5ca66b79e37d6cf3,
    0x0617baae5905c975, 0x0de58d6272b23802, 0x50b67a0cbe93737a,
    0xd8a91954a74d53f4, 0x46234555620327c2, 0x0ffdb68ed1ecf60e,
    0xa8c1912bb9821210, 0xbac95b713317e3c6, 0xb5660300ea31de06,
    0x909008878dba88b9, 0x6d64d53d0e803d9e, 0xedd0d017829b7fe7,
    0x88167b237ba66f3c, 0x9b02fa2a1c2d782d, 0x27fb52dc3da33e67,
    0x41e59b08868d9a48, 0x02e82b6b4baf3b63, 0xb4e7eaa68c93470f,
    0xab0b03bf8d9916ef, 0xa8618bb18b97699a, 0xc39af2185f56363a,
    0x1780371018e07ef9, 0x85342099363d4a9e, 0x667cc73260609c25,
    0xa829f403bc29e50c, 0xe680fffd380b016d, 0x87e38a8b79aeccd0,
    0xcf6ba66226f7b9b4, 0x785c28b889669feb, 0xf5133ca34cb9aca1,
    0x1a7a246871772331, 0x6558ac07d072495f, 0xa1caea390f893538,
    0x1aeed11773b1d4ca, 0x5f64a03fb3b0a5bc, 0x8474427ff60dae83,
    0xe697573108301952, 0xf0458f5b99e7cfcc, 0xfe5db402a8f04b5a,
    0x03710ff4bd777ac9, 0xf3f1f662995a17d6, 0x5785dbbce1502eea,
    0x790a0d68210f7a27, 0x540382ca12beb9b8, 0x847ec9136415269f,
    0xabc5727590781206, 0xda00f69ad6027876, 0x1af0a712306d24e3,
};

uint64_t verify(uint32_t round_num, uint64_t codeword) {
	round_num %= 300;
	return fibonacci(codeword, round_num * 998244353ULL,
	               codeword * 1000000007ULL) == results[round_num];
}
