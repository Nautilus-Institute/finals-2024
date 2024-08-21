#include <stdint.h>

uint64_t seeds[300] = {
    0x05e3de3684c21a23, 0x91d361ed1cd17128, 0xd4a7ea78b53845a0,
    0xed9c85babecc1ff3, 0x55cf64d85d8d2311, 0x6056644ae5cfaa52,
    0xaa66626b2aad1b63, 0x3226a849d977ffa8, 0x0690abcc8441d434,
    0xc20cd23136e5ab0a, 0x5b2dd58933266918, 0x5a6e3c7a4c6fe784,
    0xf0fa34cf630d9bc5, 0x48e41baa694759e3, 0xb65da81085d2bb59,
    0xc8c1f2093812221b, 0xedb0c8003f3de8a1, 0x527b20294615974b,
    0x5396d92212e91bd4, 0xdb83f8310e231197, 0x77c46dfa79433fb9,
    0xa97c3e00a027a225, 0xab46c9070997b9ce, 0xe4f8267b2bb997ba,
    0xbca41fc2ac7322bf, 0xeb1b052f38106ec3, 0x23a7f942573ea089,
    0xd1490d0ccbd681c0, 0xc37b5e39b1f3bdf5, 0x6a7d4d19bba10ab6,
    0xd61a1aae377e4270, 0x46dec1435557642f, 0xf0daadb21d0dadc3,
    0x038c23e7b22b657a, 0xfc962b25e7b8f2e6, 0x02729c55e8de36df,
    0x2d7768c36c50b2c2, 0x3333237034d8b83c, 0x57484a01b9c22f82,
    0x86a1e521eaa7a075, 0x6983c3ca018af556, 0xea492e0268610220,
    0xb1c96cedf735ce88, 0xcdf58c569780a5da, 0x0f804f08c328240a,
    0x415857ec1b7ea69b, 0x32649986be8f28b6, 0x32a603cfe5300c66,
    0xc57b3792e2acde13, 0xfef41130a635c133, 0x258d3778f6ab0099,
    0x64557426d880e7ad, 0xe0aa3e7c03033688, 0x197e0640b16f658b,
    0x957560bb6ceacd43, 0x0437cdddc004231a, 0xe916ab54848d1333,
    0xc1dbbac146cf8221, 0x97268daaf9685010, 0xc771c9bb5904defc,
    0xef73677bcc69aef6, 0x1c40a417ff1c2430, 0x29739972b811d444,
    0xd30d5aace6a85d7c, 0x67e4e1d306f94899, 0x24883706574bcd69,
    0x5641a09e626a2720, 0x221c8ec041b9c82c, 0x987ff3d9ea7b879d,
    0x719e9d550b158747, 0xd6bc37d761a91f4d, 0x00e66e2b965352a3,
    0x94f6acb6916f0117, 0xfbdad3b494783618, 0xf8bd2b103bec809f,
    0xba00da0c4f140e60, 0x3e88e277f9e33fdf, 0x7150a7a8bb61a8bc,
    0x45cb5af0b1340594, 0xc17a64beb422696a, 0x11766c986e2ade6b,
    0x12e988c6fb11447c, 0xf218b69c24c3014e, 0xa9593531caa3bafe,
    0xaf5b2ff0aff8e138, 0x31257aca29abcc0d, 0x9af0700ff08d217b,
    0xf9e6780fcb74763a, 0xc796a55708148545, 0x82f14d2f96ae4a8a,
    0x7d45ff3d71950776, 0xd7e5a8641cc6cc41, 0xe56118e97d2a7f47,
    0x5c49b5ca5351833b, 0x0eb8b1434c1f6a77, 0x3c909d3249dfd23a,
    0xacc8255842dbab83, 0xf9153b046973e5ec, 0x4480d9362b8055f9,
    0x6447c1becf2228e8, 0xcb8cd9c63274e263, 0x6b2348b676c7e4c5,
    0x0c49b1671aa56fa3, 0x208d42dbe494d48d, 0x08576605f45112c1,
    0xdc9dffbcf37f5fbc, 0x8211ab8b611e4f03, 0x1d13b650f669f30a,
    0x7396f484290ce1d7, 0xf2c0652c4647bbc7, 0x800fd1be98d140df,
    0xba31154874bb4311, 0x8a85ba71eb63327e, 0x9dd7967a85ce600f,
    0x2a4dca6c35e8cee5, 0x963f2770d15d9947, 0xfc8f35837b9ea469,
    0xa4d44e340ba60da2, 0x27a5c51270aec6c7, 0xba2f357b5c13ea76,
    0xc3348b5da31c5090, 0xad0077c58409a6f9, 0x7567a4419e7b9238,
    0x41a95ba6e123bb5d, 0xa128ae88bad03ebe, 0x798f5882120d0546,
    0x2ea4344aa3e31255, 0x7b9ec70d271be472, 0xcad9b03c62a5c5e7,
    0x490dec8ec655136d, 0x75dcfcf80467e7fc, 0xdbbfc675ce23e268,
    0x553577b130b4656d, 0x737d892b250123f1, 0xb0f503730ad2d63a,
    0x7c02c0cb8ef82c67, 0xf2c1e6c9fc4347e9, 0xbf777b068bd47a07,
    0x482ced3de76663ee, 0x0d2d4279169fc897, 0x58e0f5940c4ec4a9,
    0x6ae8a2b936effd71, 0xcb9fbd8f4b473be7, 0xa9dd55da7703f449,
    0xa8ea78a5d23ba48a, 0x64c0c6a90808b780, 0x234b8117d9aa4888,
    0x5f2a21e1373277e2, 0x6571f113c8acf566, 0xecb928b9674a06c0,
    0x0f28ba2b2d9dcb06, 0x7dff1e2704e67d4a, 0x68e97b330eb2e5ee,
    0x7d313f5574c5a7af, 0x790b1d9779a127cc, 0xa6bd6db515b54cc6,
    0x3da09d8397ad94d8, 0xd64b0ec475f73bdb, 0x53d76c63c44791db,
    0x30fa9115ad48560f, 0xb001c5c75bbb02eb, 0x16aa6cd06c625794,
    0x87da05f1d4731848, 0xc02648a811c41483, 0x3922dd97672dd79e,
    0x96e699228d1082ce, 0x89caecae8d3ea8d7, 0x9353bf6e9e005364,
    0x0b7a78dc20ee1a30, 0xd0f9a06044ad483b, 0xcabf70a0bb1c1aa6,
    0x31a7e7f3c15d4ee9, 0x137aff2e9f2d23b3, 0xb8ab0b49c7be099a,
    0x3c0474c61feb6785, 0xa7d75a303e27c6f3, 0x43c04074853aaa8b,
    0x908221c8ac6b3ef5, 0x0ea8fd9c9a3cff77, 0x36675aa3a1ec7ca5,
    0xc6f4d0b385bb78d3, 0xdfa367f6f5ca6075, 0x447c5232bd15ddd3,
    0x7b85ccee23da4753, 0x2d0fd001aecb061e, 0xabcf96533373afe9,
    0x8ed4a8b1698df283, 0x11f548f71add9c72, 0x5fe32836869c3ebc,
    0xcdb490d6e163cdd2, 0x7ce769277832adfa, 0x2f46af13add8493e,
    0xcca6c384f696885f, 0xcee01f22d781901c, 0x8d06ee6806d87d14,
    0x94b0391e12a9b32f, 0x9686e31c14de412c, 0x6520239f47527f04,
    0x90afcea1c5f0b520, 0x3c975f47376f67a1, 0xef62a7eab77408ce,
    0xbe095d26be388d26, 0xa15aa6e4e91fd53a, 0x257dc0c315409bc2,
    0xfa9f048c28f60ce1, 0x32f491c5f3f9953a, 0xe33049b0a38cd576,
    0x65c38427c2874a15, 0x25c309da8c15c002, 0xaecaa77130a0965f,
    0xaef760a233f614c5, 0xc14a99f1d488fc52, 0xe7dd2aa91cc9f013,
    0x43dd27fc6d2e7ae0, 0x7eaeb8d858527019, 0x2cee28c8f11490ed,
    0x5da25be4471f5e44, 0x502616a650510bcf, 0x95b3ee69937e316b,
    0x572205171775a967, 0x812b25ca2076098b, 0x5395b491acf44df1,
    0x15b95e7214891b03, 0x251b0df95e8a558d, 0xcc6d391ec58cb268,
    0xe04014757f55c119, 0x72a1d6a7dfdc6831, 0xedd1be842eb37255,
    0x077b881378f37f58, 0x60331b21881c5dda, 0x285c82ddc762c65d,
    0xd6d5939317979f17, 0xe94f72124275885e, 0x24e6aa396c61dc06,
    0x21e7effb4fa1b615, 0xd79f93b33bed328d, 0xba5b447478fb8132,
    0xf19e4400dd634378, 0x8327512766de8d95, 0x2a70bc4d8174218f,
    0x5b5f4c636bc6fef0, 0x40da7a92853a42f2, 0x6fbcc768cc1af214,
    0x4b113588cb4e5244, 0x4fb5e9d61648d01d, 0xf309db185f4987ab,
    0x579ee49108d4c2b2, 0x31b5840d1b998d5a, 0x0db2f56e872fd008,
    0xcf72d681aef8fb85, 0x1c04463d633395a8, 0x0c240e429e02ab5a,
    0xb318343f8af719c6, 0x7a39758008571203, 0x9932e4720895291f,
    0x920f9343b9a42ba3, 0x111e1d3a76776f7e, 0x68b445d12ef62161,
    0x98182d42199b5a0e, 0x95d58fba2a2ee6ff, 0x7365f2b87fef2354,
    0xa7b8671e2bc3fa7c, 0xa3cd1f0e918c6526, 0x2540deafba321c5c,
    0x854d61b1906c93f6, 0x4f9421addeadb96b, 0x19244b3276f97214,
    0xb632ed19c2b76342, 0x6b63c06d6ee69aa1, 0x2667a55efb6d8c08,
    0xd482c197bbd5a08b, 0xbefdc5e41afcc0ce, 0xfa501910ab3b6c26,
    0xc126acc709bd6333, 0xa95475f21ff1f7eb, 0xccad9fa7e0ddb722,
    0xe1233704feff38fb, 0xc012111ebbcc2542, 0xcab977b0e33bdbfa,
    0x46bf8b99f6af5259, 0x4aeec6bd652daadb, 0x15f0aaf197c9af06,
    0x95323db69e93db60, 0x24d17c278fb9bcae, 0xbf769e0de6c5f2b8,
    0x27518e0fcf161fa4, 0xdea9b6112fcfa55d, 0x52ebf66449c57176,
    0xff720e825598d2f7, 0x884647364232d3b2, 0xaddf0f1696e8f681,
    0xa2f194ed86ca32e6, 0x23df8bb4ae6a6cfa, 0x2530507edbb10e07,
    0x774fde887b25cdd0, 0x3edfdadfdf2a52eb, 0xb724eebd4a5b09cc,
    0x008f163e02fbf2f9, 0xaa3c737fa0c9b0d1, 0x17e333251a9e1fb3};

uint64_t generate(uint32_t round_num) {
	round_num %= 300;
	return seeds[round_num];
}
