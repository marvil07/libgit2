
#include "test_lib.h"
#include "test_helpers.h"
#include <git2/odb.h>

/*
 * read loose objects from the object directory. The objects are
 * written using the "in-pack" object encoding, using an zlib
 * compression level of Z_DEFAULT_COMPRESSION (6). See also
 * t0202-readloose.c.
 *
 * Note that the tree and tag objects are not actually stored in
 * the "in-pack" format. This is due to a bug in git v1.5.2, since
 * git-write-tree and git-mktag did not call git_default_config()
 * and, therefore, did not honor the "core.legacyheaders" config
 * variable.
 */

static char *odb_dir = "test-objects";

/* commit == 3d7f8a6af076c8c3f20071a8935cdbe8228594d1 */
static unsigned char commit_bytes[] = {
    0x92, 0x16, 0x78, 0x9c, 0x85, 0x90, 0x3d, 0x6e,
    0xc3, 0x30, 0x0c, 0x85, 0x77, 0x9d, 0x82, 0x7b,
    0xeb, 0xc0, 0x52, 0x53, 0x58, 0x2e, 0x82, 0xa2,
    0x41, 0xe7, 0x20, 0x43, 0xd3, 0x03, 0x48, 0x11,
    0x65, 0x0b, 0xb0, 0x28, 0x43, 0x3f, 0x40, 0x7c,
    0xfb, 0x28, 0x70, 0x61, 0x74, 0x6a, 0xc9, 0xe5,
    0xf1, 0x91, 0xdf, 0x1b, 0x98, 0x23, 0x22, 0x18,
    0x6b, 0x85, 0x51, 0x7d, 0xab, 0xc5, 0xeb, 0x1e,
    0xb9, 0x46, 0x2d, 0x65, 0x6f, 0xb8, 0xad, 0x2d,
    0xa4, 0xd4, 0xc8, 0x65, 0xfb, 0xd2, 0x49, 0x61,
    0x2c, 0x53, 0x25, 0x8f, 0x21, 0xc2, 0x11, 0xbe,
    0xe1, 0xf2, 0x10, 0x87, 0xd5, 0xf8, 0xc0, 0x9b,
    0xf2, 0xf3, 0x84, 0xbb, 0x6b, 0xf0, 0xef, 0xc0,
    0x85, 0xe8, 0x24, 0xdf, 0x8b, 0xbe, 0x83, 0xa7,
    0xb6, 0x16, 0xab, 0xae, 0x77, 0x39, 0x63, 0x84,
    0x4f, 0x38, 0xc3, 0x69, 0x95, 0x87, 0xcd, 0xfd,
    0x87, 0x66, 0x47, 0x08, 0x84, 0xcd, 0xe4, 0x08,
    0x61, 0x65, 0x20, 0x15, 0xef, 0x55, 0x5c, 0x18,
    0xbb, 0x8c, 0x08, 0x3a, 0x98, 0x05, 0x82, 0x85,
    0x3c, 0x6e, 0x7b, 0x8f, 0x29, 0xa9, 0x01, 0x9f,
    0xeb, 0x4c, 0x59, 0x39, 0x72, 0x34, 0x80, 0x2d,
    0xb1, 0x5e, 0x44, 0xc0, 0xdb, 0x3c, 0x29, 0x52,
    0xd9, 0x05, 0x62, 0x3f, 0xd4, 0x5c, 0xe2, 0x1c,
    0x12, 0x6e, 0x21, 0xa3, 0xa2, 0x01, 0x13, 0x38,
    0xca, 0x31, 0x98, 0x72, 0x45, 0x03, 0x7a, 0xf9,
    0x15, 0xbf, 0x63, 0xec, 0xcb, 0x0d, 0x84, 0xa6,
    0x09, 0xb6, 0xd1, 0xcb, 0xdb, 0xdf, 0xef, 0x60,
    0x77, 0x51, 0x90, 0x74, 0xf0,
};

static unsigned char commit_data[] = {
    0x74, 0x72, 0x65, 0x65, 0x20, 0x64, 0x66, 0x66,
    0x32, 0x64, 0x61, 0x39, 0x30, 0x62, 0x32, 0x35,
    0x34, 0x65, 0x31, 0x62, 0x65, 0x62, 0x38, 0x38,
    0x39, 0x64, 0x31, 0x66, 0x31, 0x66, 0x31, 0x32,
    0x38, 0x38, 0x62, 0x65, 0x31, 0x38, 0x30, 0x33,
    0x37, 0x38, 0x32, 0x64, 0x66, 0x0a, 0x61, 0x75,
    0x74, 0x68, 0x6f, 0x72, 0x20, 0x41, 0x20, 0x55,
    0x20, 0x54, 0x68, 0x6f, 0x72, 0x20, 0x3c, 0x61,
    0x75, 0x74, 0x68, 0x6f, 0x72, 0x40, 0x65, 0x78,
    0x61, 0x6d, 0x70, 0x6c, 0x65, 0x2e, 0x63, 0x6f,
    0x6d, 0x3e, 0x20, 0x31, 0x32, 0x32, 0x37, 0x38,
    0x31, 0x34, 0x32, 0x39, 0x37, 0x20, 0x2b, 0x30,
    0x30, 0x30, 0x30, 0x0a, 0x63, 0x6f, 0x6d, 0x6d,
    0x69, 0x74, 0x74, 0x65, 0x72, 0x20, 0x43, 0x20,
    0x4f, 0x20, 0x4d, 0x69, 0x74, 0x74, 0x65, 0x72,
    0x20, 0x3c, 0x63, 0x6f, 0x6d, 0x6d, 0x69, 0x74,
    0x74, 0x65, 0x72, 0x40, 0x65, 0x78, 0x61, 0x6d,
    0x70, 0x6c, 0x65, 0x2e, 0x63, 0x6f, 0x6d, 0x3e,
    0x20, 0x31, 0x32, 0x32, 0x37, 0x38, 0x31, 0x34,
    0x32, 0x39, 0x37, 0x20, 0x2b, 0x30, 0x30, 0x30,
    0x30, 0x0a, 0x0a, 0x41, 0x20, 0x6f, 0x6e, 0x65,
    0x2d, 0x6c, 0x69, 0x6e, 0x65, 0x20, 0x63, 0x6f,
    0x6d, 0x6d, 0x69, 0x74, 0x20, 0x73, 0x75, 0x6d,
    0x6d, 0x61, 0x72, 0x79, 0x0a, 0x0a, 0x54, 0x68,
    0x65, 0x20, 0x62, 0x6f, 0x64, 0x79, 0x20, 0x6f,
    0x66, 0x20, 0x74, 0x68, 0x65, 0x20, 0x63, 0x6f,
    0x6d, 0x6d, 0x69, 0x74, 0x20, 0x6d, 0x65, 0x73,
    0x73, 0x61, 0x67, 0x65, 0x2c, 0x20, 0x63, 0x6f,
    0x6e, 0x74, 0x61, 0x69, 0x6e, 0x69, 0x6e, 0x67,
    0x20, 0x66, 0x75, 0x72, 0x74, 0x68, 0x65, 0x72,
    0x20, 0x65, 0x78, 0x70, 0x6c, 0x61, 0x6e, 0x61,
    0x74, 0x69, 0x6f, 0x6e, 0x0a, 0x6f, 0x66, 0x20,
    0x74, 0x68, 0x65, 0x20, 0x70, 0x75, 0x72, 0x70,
    0x6f, 0x73, 0x65, 0x20, 0x6f, 0x66, 0x20, 0x74,
    0x68, 0x65, 0x20, 0x63, 0x68, 0x61, 0x6e, 0x67,
    0x65, 0x73, 0x20, 0x69, 0x6e, 0x74, 0x72, 0x6f,
    0x64, 0x75, 0x63, 0x65, 0x64, 0x20, 0x62, 0x79,
    0x20, 0x74, 0x68, 0x65, 0x20, 0x63, 0x6f, 0x6d,
    0x6d, 0x69, 0x74, 0x2e, 0x0a, 0x0a, 0x53, 0x69,
    0x67, 0x6e, 0x65, 0x64, 0x2d, 0x6f, 0x66, 0x2d,
    0x62, 0x79, 0x3a, 0x20, 0x41, 0x20, 0x55, 0x20,
    0x54, 0x68, 0x6f, 0x72, 0x20, 0x3c, 0x61, 0x75,
    0x74, 0x68, 0x6f, 0x72, 0x40, 0x65, 0x78, 0x61,
    0x6d, 0x70, 0x6c, 0x65, 0x2e, 0x63, 0x6f, 0x6d,
    0x3e, 0x0a,
};

static object_data commit = {
    commit_bytes,
    sizeof(commit_bytes),
    "3d7f8a6af076c8c3f20071a8935cdbe8228594d1",
    "commit",
    "test-objects/3d",
    "test-objects/3d/7f8a6af076c8c3f20071a8935cdbe8228594d1",
    commit_data,
    sizeof(commit_data),
};

/* tree == dff2da90b254e1beb889d1f1f1288be1803782df */
static unsigned char tree_bytes[] = {
    0x78, 0x9c, 0x2b, 0x29, 0x4a, 0x4d, 0x55, 0x30,
    0x34, 0x32, 0x63, 0x30, 0x34, 0x30, 0x30, 0x33,
    0x31, 0x51, 0xc8, 0xcf, 0x4b, 0x65, 0xe8, 0x16,
    0xae, 0x98, 0x58, 0x29, 0xff, 0x32, 0x53, 0x7d,
    0x6d, 0xc5, 0x33, 0x6f, 0xae, 0xb5, 0xd5, 0xf7,
    0x2e, 0x74, 0xdf, 0x81, 0x4a, 0x17, 0xe7, 0xe7,
    0xa6, 0x32, 0xfc, 0x6d, 0x31, 0xd8, 0xd3, 0xe6,
    0xf3, 0xe7, 0xea, 0x47, 0xbe, 0xd0, 0x09, 0x3f,
    0x96, 0xb8, 0x3f, 0x90, 0x9e, 0xa2, 0xfd, 0x0f,
    0x2a, 0x5f, 0x52, 0x9e, 0xcf, 0x50, 0x31, 0x43,
    0x52, 0x29, 0xd1, 0x5a, 0xeb, 0x77, 0x82, 0x2a,
    0x8b, 0xfe, 0xb7, 0xbd, 0xed, 0x5d, 0x07, 0x67,
    0xfa, 0xb5, 0x42, 0xa5, 0xab, 0x52, 0x8b, 0xf2,
    0x19, 0x9e, 0xcd, 0x7d, 0x34, 0x7b, 0xd3, 0xc5,
    0x6b, 0xce, 0xde, 0xdd, 0x9a, 0xeb, 0xca, 0xa3,
    0x6e, 0x1c, 0x7a, 0xd2, 0x13, 0x3c, 0x11, 0x00,
    0xe2, 0xaa, 0x38, 0x57,
};

static unsigned char tree_data[] = {
    0x31, 0x30, 0x30, 0x36, 0x34, 0x34, 0x20, 0x6f,
    0x6e, 0x65, 0x00, 0x8b, 0x13, 0x78, 0x91, 0x79,
    0x1f, 0xe9, 0x69, 0x27, 0xad, 0x78, 0xe6, 0x4b,
    0x0a, 0xad, 0x7b, 0xde, 0xd0, 0x8b, 0xdc, 0x31,
    0x30, 0x30, 0x36, 0x34, 0x34, 0x20, 0x73, 0x6f,
    0x6d, 0x65, 0x00, 0xfd, 0x84, 0x30, 0xbc, 0x86,
    0x4c, 0xfc, 0xd5, 0xf1, 0x0e, 0x55, 0x90, 0xf8,
    0xa4, 0x47, 0xe0, 0x1b, 0x94, 0x2b, 0xfe, 0x31,
    0x30, 0x30, 0x36, 0x34, 0x34, 0x20, 0x74, 0x77,
    0x6f, 0x00, 0x78, 0x98, 0x19, 0x22, 0x61, 0x3b,
    0x2a, 0xfb, 0x60, 0x25, 0x04, 0x2f, 0xf6, 0xbd,
    0x87, 0x8a, 0xc1, 0x99, 0x4e, 0x85, 0x31, 0x30,
    0x30, 0x36, 0x34, 0x34, 0x20, 0x7a, 0x65, 0x72,
    0x6f, 0x00, 0xe6, 0x9d, 0xe2, 0x9b, 0xb2, 0xd1,
    0xd6, 0x43, 0x4b, 0x8b, 0x29, 0xae, 0x77, 0x5a,
    0xd8, 0xc2, 0xe4, 0x8c, 0x53, 0x91,
};

static object_data tree = {
    tree_bytes,
    sizeof(tree_bytes),
    "dff2da90b254e1beb889d1f1f1288be1803782df",
    "tree",
    "test-objects/df",
    "test-objects/df/f2da90b254e1beb889d1f1f1288be1803782df",
    tree_data,
    sizeof(tree_data),
};

/* tag == 09d373e1dfdc16b129ceec6dd649739911541e05 */
static unsigned char tag_bytes[] = {
    0x78, 0x9c, 0x35, 0x8e, 0xcb, 0x0a, 0xc2, 0x30,
    0x10, 0x45, 0x5d, 0xe7, 0x2b, 0x66, 0x2f, 0x84,
    0x3c, 0xda, 0x66, 0x0a, 0x22, 0x82, 0x6b, 0x71,
    0xe3, 0x0f, 0xa4, 0xe9, 0xa4, 0x0f, 0x5a, 0x52,
    0xda, 0x41, 0xf4, 0xef, 0x4d, 0x51, 0x87, 0x59,
    0x1c, 0x2e, 0x97, 0x33, 0xc3, 0xbe, 0x03, 0xed,
    0xec, 0x21, 0x35, 0x23, 0x05, 0x06, 0xdb, 0xba,
    0x88, 0xbe, 0xf2, 0x51, 0xb9, 0x2a, 0x60, 0xb0,
    0xd1, 0x28, 0xe5, 0xb4, 0xc7, 0xda, 0x96, 0xa1,
    0x6d, 0x08, 0x8d, 0xc1, 0xb2, 0x2e, 0x5a, 0x2d,
    0xf8, 0xbd, 0x10, 0x84, 0x34, 0xcf, 0x03, 0x0b,
    0xce, 0x8e, 0xa7, 0x92, 0x4a, 0xea, 0x1d, 0x3b,
    0x5a, 0xe1, 0x0a, 0x77, 0xb8, 0x0d, 0xcc, 0x19,
    0x4f, 0xdf, 0x52, 0xc6, 0x0b, 0xbd, 0xfc, 0xbc,
    0x4c, 0x24, 0x73, 0x72, 0x06, 0x6d, 0x8c, 0x43,
    0x5d, 0x98, 0xda, 0xc1, 0x51, 0xe5, 0x11, 0xe2,
    0xd1, 0x0f, 0x1b, 0xe4, 0xe5, 0x9e, 0x60, 0x57,
    0xfe, 0x5e, 0x8a, 0x69, 0x85, 0x95, 0x26, 0xf2,
    0x1b, 0xfd, 0xaf, 0x7c, 0x00, 0x42, 0x9a, 0x36,
    0xb1,
};

static unsigned char tag_data[] = {
    0x6f, 0x62, 0x6a, 0x65, 0x63, 0x74, 0x20, 0x33,
    0x64, 0x37, 0x66, 0x38, 0x61, 0x36, 0x61, 0x66,
    0x30, 0x37, 0x36, 0x63, 0x38, 0x63, 0x33, 0x66,
    0x32, 0x30, 0x30, 0x37, 0x31, 0x61, 0x38, 0x39,
    0x33, 0x35, 0x63, 0x64, 0x62, 0x65, 0x38, 0x32,
    0x32, 0x38, 0x35, 0x39, 0x34, 0x64, 0x31, 0x0a,
    0x74, 0x79, 0x70, 0x65, 0x20, 0x63, 0x6f, 0x6d,
    0x6d, 0x69, 0x74, 0x0a, 0x74, 0x61, 0x67, 0x20,
    0x76, 0x30, 0x2e, 0x30, 0x2e, 0x31, 0x0a, 0x74,
    0x61, 0x67, 0x67, 0x65, 0x72, 0x20, 0x43, 0x20,
    0x4f, 0x20, 0x4d, 0x69, 0x74, 0x74, 0x65, 0x72,
    0x20, 0x3c, 0x63, 0x6f, 0x6d, 0x6d, 0x69, 0x74,
    0x74, 0x65, 0x72, 0x40, 0x65, 0x78, 0x61, 0x6d,
    0x70, 0x6c, 0x65, 0x2e, 0x63, 0x6f, 0x6d, 0x3e,
    0x20, 0x31, 0x32, 0x32, 0x37, 0x38, 0x31, 0x34,
    0x32, 0x39, 0x37, 0x20, 0x2b, 0x30, 0x30, 0x30,
    0x30, 0x0a, 0x0a, 0x54, 0x68, 0x69, 0x73, 0x20,
    0x69, 0x73, 0x20, 0x74, 0x68, 0x65, 0x20, 0x74,
    0x61, 0x67, 0x20, 0x6f, 0x62, 0x6a, 0x65, 0x63,
    0x74, 0x20, 0x66, 0x6f, 0x72, 0x20, 0x72, 0x65,
    0x6c, 0x65, 0x61, 0x73, 0x65, 0x20, 0x76, 0x30,
    0x2e, 0x30, 0x2e, 0x31, 0x0a,
};

static object_data tag = {
    tag_bytes,
    sizeof(tag_bytes),
    "09d373e1dfdc16b129ceec6dd649739911541e05",
    "tag",
    "test-objects/09",
    "test-objects/09/d373e1dfdc16b129ceec6dd649739911541e05",
    tag_data,
    sizeof(tag_data),
};

/* zero == e69de29bb2d1d6434b8b29ae775ad8c2e48c5391 */
static unsigned char zero_bytes[] = {
    0x30, 0x78, 0x9c, 0x03, 0x00, 0x00, 0x00, 0x00,
    0x01,
};

static unsigned char zero_data[] = {
    0x00  /* dummy data */
};

static object_data zero = {
    zero_bytes,
    sizeof(zero_bytes),
    "e69de29bb2d1d6434b8b29ae775ad8c2e48c5391",
    "blob",
    "test-objects/e6",
    "test-objects/e6/9de29bb2d1d6434b8b29ae775ad8c2e48c5391",
    zero_data,
    0,
};

/* one == 8b137891791fe96927ad78e64b0aad7bded08bdc */
static unsigned char one_bytes[] = {
    0x31, 0x78, 0x9c, 0xe3, 0x02, 0x00, 0x00, 0x0b,
    0x00, 0x0b,
};

static unsigned char one_data[] = {
    0x0a,
};

static object_data one = {
    one_bytes,
    sizeof(one_bytes),
    "8b137891791fe96927ad78e64b0aad7bded08bdc",
    "blob",
    "test-objects/8b",
    "test-objects/8b/137891791fe96927ad78e64b0aad7bded08bdc",
    one_data,
    sizeof(one_data),
};

/* two == 78981922613b2afb6025042ff6bd878ac1994e85 */
static unsigned char two_bytes[] = {
    0x32, 0x78, 0x9c, 0x4b, 0xe4, 0x02, 0x00, 0x00,
    0xce, 0x00, 0x6c,
};

static unsigned char two_data[] = {
    0x61, 0x0a,
};

static object_data two = {
    two_bytes,
    sizeof(two_bytes),
    "78981922613b2afb6025042ff6bd878ac1994e85",
    "blob",
    "test-objects/78",
    "test-objects/78/981922613b2afb6025042ff6bd878ac1994e85",
    two_data,
    sizeof(two_data),
};

/* some == fd8430bc864cfcd5f10e5590f8a447e01b942bfe */
static unsigned char some_bytes[] = {
    0xb1, 0x49, 0x78, 0x9c, 0x85, 0x53, 0xc1, 0x4e,
    0xe3, 0x30, 0x10, 0xbd, 0xef, 0x57, 0xcc, 0x71,
    0x17, 0x65, 0x81, 0xb2, 0xda, 0x53, 0x4f, 0x01,
    0x51, 0x88, 0x04, 0x6d, 0x95, 0xa4, 0x42, 0x3d,
    0xba, 0xf1, 0x84, 0x8c, 0x70, 0xec, 0xc8, 0x76,
    0x28, 0xf9, 0xfb, 0x9d, 0x31, 0x2d, 0xb0, 0x5a,
    0x56, 0x9c, 0xa2, 0xd8, 0xf3, 0xde, 0xbc, 0x79,
    0xf3, 0x7c, 0x76, 0xf2, 0x0d, 0x4e, 0xa0, 0xee,
    0x28, 0x40, 0x4b, 0x06, 0x41, 0xbe, 0x1e, 0x11,
    0x82, 0x6b, 0xe3, 0x5e, 0x79, 0x9c, 0xc3, 0xe4,
    0x46, 0x68, 0x94, 0x05, 0x8f, 0x9a, 0x42, 0xf4,
    0xb4, 0x1b, 0x23, 0x97, 0x45, 0x50, 0x56, 0x9f,
    0x39, 0x0f, 0xbd, 0xd3, 0xd4, 0x4e, 0x42, 0xc2,
    0x67, 0xa3, 0xd5, 0xe8, 0x21, 0x76, 0x08, 0x11,
    0x7d, 0x1f, 0xc0, 0xb5, 0xe9, 0xe7, 0x66, 0xb9,
    0x81, 0x1b, 0xb4, 0xe8, 0x95, 0x81, 0xf5, 0xb8,
    0x33, 0xd4, 0xc0, 0x1d, 0x35, 0x68, 0x03, 0x66,
    0xf0, 0x8c, 0x3e, 0x90, 0xb3, 0x70, 0x91, 0x09,
    0x87, 0x0a, 0x30, 0x48, 0x41, 0xe8, 0x50, 0xc3,
    0x6e, 0x4a, 0xe8, 0x85, 0xe8, 0xa9, 0x0e, 0x7a,
    0x60, 0xe1, 0xb8, 0x89, 0x8a, 0x0c, 0x39, 0x65,
    0x80, 0x60, 0x0a, 0x0b, 0x4a, 0x6b, 0x92, 0x23,
    0x88, 0x2e, 0x41, 0x06, 0xee, 0x4e, 0x41, 0x78,
    0x03, 0x90, 0xfd, 0x4a, 0x83, 0x90, 0x48, 0x89,
    0x1a, 0x63, 0xe7, 0x7c, 0x80, 0x47, 0x7a, 0xc6,
    0x34, 0xf6, 0x68, 0x0d, 0xf5, 0x14, 0x59, 0xca,
    0x3b, 0xa1, 0xb4, 0x30, 0x64, 0x9f, 0x12, 0xa2,
    0x71, 0xfd, 0xc0, 0xae, 0x69, 0x61, 0x38, 0x0e,
    0x92, 0x66, 0x7e, 0xb3, 0xd3, 0x72, 0x39, 0x57,
    0xed, 0xc8, 0x26, 0xcd, 0x01, 0xf6, 0x14, 0x3b,
    0x70, 0x0c, 0xf6, 0x30, 0x78, 0xf7, 0xe8, 0x55,
    0x1f, 0x5e, 0x27, 0xb7, 0x5a, 0xa8, 0x3f, 0x78,
    0xcc, 0x62, 0x02, 0xfe, 0x0b, 0x76, 0xa3, 0x78,
    0x3f, 0xf1, 0x3e, 0xa4, 0xb2, 0x91, 0x0b, 0xc1,
    0x73, 0x1d, 0xd9, 0x47, 0x5e, 0x9e, 0xeb, 0x93,
    0xb4, 0x91, 0xb1, 0x1f, 0xa5, 0x9c, 0x02, 0x7c,
    0xaf, 0xc5, 0x87, 0x4f, 0x3d, 0x10, 0x86, 0x0f,
    0x84, 0x01, 0xb4, 0x03, 0x35, 0x0c, 0x66, 0x12,
    0xfb, 0x5e, 0xd5, 0xf2, 0xf5, 0x80, 0x4d, 0x0c,
    0x73, 0x68, 0x79, 0xed, 0xf8, 0xa2, 0xfa, 0xc1,
    0xf0, 0xfe, 0xf8, 0x6e, 0xe2, 0xe6, 0x3c, 0xbd,
    0x70, 0xa4, 0x34, 0x50, 0x93, 0xe4, 0x1e, 0xb7,
    0x2f, 0xdd, 0xb3, 0x34, 0xdf, 0xdb, 0x70, 0x72,
    0xbb, 0xef, 0xd0, 0x82, 0x75, 0x31, 0xb9, 0xc9,
    0x16, 0x8b, 0x55, 0xc9, 0x88, 0xc3, 0xc8, 0x7c,
    0x84, 0x2f, 0xd8, 0x8c, 0x51, 0xed, 0x58, 0xfd,
    0x8f, 0xc3, 0xb2, 0xff, 0x4a, 0xea, 0xbb, 0x59,
    0xfa, 0xb8, 0xe6, 0xce, 0x0d, 0xe2, 0x9c, 0x8a,
    0x12, 0xc7, 0x3d, 0x19, 0x03, 0xbb, 0xe4, 0x45,
    0x3b, 0x9a, 0x0c, 0xb8, 0x52, 0x38, 0x1e, 0x8a,
    0xfa, 0x76, 0xb5, 0xa9, 0x21, 0x5f, 0x6e, 0xe1,
    0x21, 0x2f, 0xcb, 0x7c, 0x59, 0x6f, 0xe7, 0x6f,
    0xde, 0xe2, 0x33, 0xbe, 0x52, 0x11, 0x0f, 0x48,
    0xcc, 0xcc, 0xb1, 0xf3, 0xca, 0xc6, 0x89, 0xe7,
    0x11, 0xf4, 0xfd, 0x75, 0x79, 0x75, 0xcb, 0x90,
    0xfc, 0xb2, 0xb8, 0x2b, 0xea, 0x2d, 0xb0, 0x19,
    0x8b, 0xa2, 0x5e, 0x5e, 0x57, 0x15, 0x2c, 0x56,
    0x25, 0xe4, 0xb0, 0xce, 0xcb, 0xba, 0xb8, 0xda,
    0xdc, 0xe5, 0x25, 0xac, 0x37, 0xe5, 0x7a, 0x55,
    0x5d, 0xb3, 0xf7, 0x15, 0xe2, 0x31, 0x86, 0x42,
    0xf2, 0xf9, 0x16, 0x92, 0xb3, 0xbd, 0xe3, 0x94,
    0x6b, 0x8c, 0x8a, 0x4c, 0x38, 0x46, 0x7c, 0xcb,
    0x61, 0x0c, 0xac, 0xce, 0x68, 0xe8, 0x14, 0x87,
    0xd3, 0x63, 0x83, 0x1c, 0x52, 0x9d, 0xcc, 0x1a,
    0xa6, 0xaf, 0xdf, 0x59, 0x32, 0xd6, 0x38, 0x0e,
    0x48, 0xca, 0x5f, 0x4a, 0xc5, 0x21, 0x7e, 0x73,
    0x08, 0x07, 0x6d, 0xc9, 0xd6, 0xab, 0xd5, 0x7a,
    0x5b, 0x2c, 0x6f, 0x58, 0x72, 0xd1, 0xca, 0x7a,
    0x32, 0xd8, 0x7b, 0x92, 0x34, 0xba, 0xe3, 0x33,
    0xf9, 0xdf, 0x7b, 0xcc, 0xe0, 0xf7, 0x8c, 0x2f,
    0x95, 0x7d, 0xe2, 0x8d, 0x42, 0x15, 0xb9, 0x8c,
    0xd1, 0x0b, 0x6a, 0xb9, 0xe1, 0xc2, 0x38, 0xe7,
    0x53, 0xce, 0x2f, 0x5d, 0x88, 0x52, 0x7b, 0x9f,
    0xc3, 0xf9, 0xc5, 0x6c, 0x76, 0xfe, 0x73, 0xf6,
    0xeb, 0x7c, 0x96, 0xc1, 0xa6, 0xca, 0x65, 0xd8,
    0xb3, 0x6f, 0x7f, 0x00, 0x5d, 0x59, 0x88, 0xc3,
};

static unsigned char some_data[] = {
    0x2f, 0x2a, 0x0a, 0x20, 0x2a, 0x20, 0x54, 0x68,
    0x69, 0x73, 0x20, 0x66, 0x69, 0x6c, 0x65, 0x20,
    0x69, 0x73, 0x20, 0x66, 0x72, 0x65, 0x65, 0x20,
    0x73, 0x6f, 0x66, 0x74, 0x77, 0x61, 0x72, 0x65,
    0x3b, 0x20, 0x79, 0x6f, 0x75, 0x20, 0x63, 0x61,
    0x6e, 0x20, 0x72, 0x65, 0x64, 0x69, 0x73, 0x74,
    0x72, 0x69, 0x62, 0x75, 0x74, 0x65, 0x20, 0x69,
    0x74, 0x20, 0x61, 0x6e, 0x64, 0x2f, 0x6f, 0x72,
    0x20, 0x6d, 0x6f, 0x64, 0x69, 0x66, 0x79, 0x0a,
    0x20, 0x2a, 0x20, 0x69, 0x74, 0x20, 0x75, 0x6e,
    0x64, 0x65, 0x72, 0x20, 0x74, 0x68, 0x65, 0x20,
    0x74, 0x65, 0x72, 0x6d, 0x73, 0x20, 0x6f, 0x66,
    0x20, 0x74, 0x68, 0x65, 0x20, 0x47, 0x4e, 0x55,
    0x20, 0x47, 0x65, 0x6e, 0x65, 0x72, 0x61, 0x6c,
    0x20, 0x50, 0x75, 0x62, 0x6c, 0x69, 0x63, 0x20,
    0x4c, 0x69, 0x63, 0x65, 0x6e, 0x73, 0x65, 0x2c,
    0x20, 0x76, 0x65, 0x72, 0x73, 0x69, 0x6f, 0x6e,
    0x20, 0x32, 0x2c, 0x0a, 0x20, 0x2a, 0x20, 0x61,
    0x73, 0x20, 0x70, 0x75, 0x62, 0x6c, 0x69, 0x73,
    0x68, 0x65, 0x64, 0x20, 0x62, 0x79, 0x20, 0x74,
    0x68, 0x65, 0x20, 0x46, 0x72, 0x65, 0x65, 0x20,
    0x53, 0x6f, 0x66, 0x74, 0x77, 0x61, 0x72, 0x65,
    0x20, 0x46, 0x6f, 0x75, 0x6e, 0x64, 0x61, 0x74,
    0x69, 0x6f, 0x6e, 0x2e, 0x0a, 0x20, 0x2a, 0x0a,
    0x20, 0x2a, 0x20, 0x49, 0x6e, 0x20, 0x61, 0x64,
    0x64, 0x69, 0x74, 0x69, 0x6f, 0x6e, 0x20, 0x74,
    0x6f, 0x20, 0x74, 0x68, 0x65, 0x20, 0x70, 0x65,
    0x72, 0x6d, 0x69, 0x73, 0x73, 0x69, 0x6f, 0x6e,
    0x73, 0x20, 0x69, 0x6e, 0x20, 0x74, 0x68, 0x65,
    0x20, 0x47, 0x4e, 0x55, 0x20, 0x47, 0x65, 0x6e,
    0x65, 0x72, 0x61, 0x6c, 0x20, 0x50, 0x75, 0x62,
    0x6c, 0x69, 0x63, 0x20, 0x4c, 0x69, 0x63, 0x65,
    0x6e, 0x73, 0x65, 0x2c, 0x0a, 0x20, 0x2a, 0x20,
    0x74, 0x68, 0x65, 0x20, 0x61, 0x75, 0x74, 0x68,
    0x6f, 0x72, 0x73, 0x20, 0x67, 0x69, 0x76, 0x65,
    0x20, 0x79, 0x6f, 0x75, 0x20, 0x75, 0x6e, 0x6c,
    0x69, 0x6d, 0x69, 0x74, 0x65, 0x64, 0x20, 0x70,
    0x65, 0x72, 0x6d, 0x69, 0x73, 0x73, 0x69, 0x6f,
    0x6e, 0x20, 0x74, 0x6f, 0x20, 0x6c, 0x69, 0x6e,
    0x6b, 0x20, 0x74, 0x68, 0x65, 0x20, 0x63, 0x6f,
    0x6d, 0x70, 0x69, 0x6c, 0x65, 0x64, 0x0a, 0x20,
    0x2a, 0x20, 0x76, 0x65, 0x72, 0x73, 0x69, 0x6f,
    0x6e, 0x20, 0x6f, 0x66, 0x20, 0x74, 0x68, 0x69,
    0x73, 0x20, 0x66, 0x69, 0x6c, 0x65, 0x20, 0x69,
    0x6e, 0x74, 0x6f, 0x20, 0x63, 0x6f, 0x6d, 0x62,
    0x69, 0x6e, 0x61, 0x74, 0x69, 0x6f, 0x6e, 0x73,
    0x20, 0x77, 0x69, 0x74, 0x68, 0x20, 0x6f, 0x74,
    0x68, 0x65, 0x72, 0x20, 0x70, 0x72, 0x6f, 0x67,
    0x72, 0x61, 0x6d, 0x73, 0x2c, 0x0a, 0x20, 0x2a,
    0x20, 0x61, 0x6e, 0x64, 0x20, 0x74, 0x6f, 0x20,
    0x64, 0x69, 0x73, 0x74, 0x72, 0x69, 0x62, 0x75,
    0x74, 0x65, 0x20, 0x74, 0x68, 0x6f, 0x73, 0x65,
    0x20, 0x63, 0x6f, 0x6d, 0x62, 0x69, 0x6e, 0x61,
    0x74, 0x69, 0x6f, 0x6e, 0x73, 0x20, 0x77, 0x69,
    0x74, 0x68, 0x6f, 0x75, 0x74, 0x20, 0x61, 0x6e,
    0x79, 0x20, 0x72, 0x65, 0x73, 0x74, 0x72, 0x69,
    0x63, 0x74, 0x69, 0x6f, 0x6e, 0x0a, 0x20, 0x2a,
    0x20, 0x63, 0x6f, 0x6d, 0x69, 0x6e, 0x67, 0x20,
    0x66, 0x72, 0x6f, 0x6d, 0x20, 0x74, 0x68, 0x65,
    0x20, 0x75, 0x73, 0x65, 0x20, 0x6f, 0x66, 0x20,
    0x74, 0x68, 0x69, 0x73, 0x20, 0x66, 0x69, 0x6c,
    0x65, 0x2e, 0x20, 0x20, 0x28, 0x54, 0x68, 0x65,
    0x20, 0x47, 0x65, 0x6e, 0x65, 0x72, 0x61, 0x6c,
    0x20, 0x50, 0x75, 0x62, 0x6c, 0x69, 0x63, 0x20,
    0x4c, 0x69, 0x63, 0x65, 0x6e, 0x73, 0x65, 0x0a,
    0x20, 0x2a, 0x20, 0x72, 0x65, 0x73, 0x74, 0x72,
    0x69, 0x63, 0x74, 0x69, 0x6f, 0x6e, 0x73, 0x20,
    0x64, 0x6f, 0x20, 0x61, 0x70, 0x70, 0x6c, 0x79,
    0x20, 0x69, 0x6e, 0x20, 0x6f, 0x74, 0x68, 0x65,
    0x72, 0x20, 0x72, 0x65, 0x73, 0x70, 0x65, 0x63,
    0x74, 0x73, 0x3b, 0x20, 0x66, 0x6f, 0x72, 0x20,
    0x65, 0x78, 0x61, 0x6d, 0x70, 0x6c, 0x65, 0x2c,
    0x20, 0x74, 0x68, 0x65, 0x79, 0x20, 0x63, 0x6f,
    0x76, 0x65, 0x72, 0x0a, 0x20, 0x2a, 0x20, 0x6d,
    0x6f, 0x64, 0x69, 0x66, 0x69, 0x63, 0x61, 0x74,
    0x69, 0x6f, 0x6e, 0x20, 0x6f, 0x66, 0x20, 0x74,
    0x68, 0x65, 0x20, 0x66, 0x69, 0x6c, 0x65, 0x2c,
    0x20, 0x61, 0x6e, 0x64, 0x20, 0x64, 0x69, 0x73,
    0x74, 0x72, 0x69, 0x62, 0x75, 0x74, 0x69, 0x6f,
    0x6e, 0x20, 0x77, 0x68, 0x65, 0x6e, 0x20, 0x6e,
    0x6f, 0x74, 0x20, 0x6c, 0x69, 0x6e, 0x6b, 0x65,
    0x64, 0x20, 0x69, 0x6e, 0x74, 0x6f, 0x0a, 0x20,
    0x2a, 0x20, 0x61, 0x20, 0x63, 0x6f, 0x6d, 0x62,
    0x69, 0x6e, 0x65, 0x64, 0x20, 0x65, 0x78, 0x65,
    0x63, 0x75, 0x74, 0x61, 0x62, 0x6c, 0x65, 0x2e,
    0x29, 0x0a, 0x20, 0x2a, 0x0a, 0x20, 0x2a, 0x20,
    0x54, 0x68, 0x69, 0x73, 0x20, 0x66, 0x69, 0x6c,
    0x65, 0x20, 0x69, 0x73, 0x20, 0x64, 0x69, 0x73,
    0x74, 0x72, 0x69, 0x62, 0x75, 0x74, 0x65, 0x64,
    0x20, 0x69, 0x6e, 0x20, 0x74, 0x68, 0x65, 0x20,
    0x68, 0x6f, 0x70, 0x65, 0x20, 0x74, 0x68, 0x61,
    0x74, 0x20, 0x69, 0x74, 0x20, 0x77, 0x69, 0x6c,
    0x6c, 0x20, 0x62, 0x65, 0x20, 0x75, 0x73, 0x65,
    0x66, 0x75, 0x6c, 0x2c, 0x20, 0x62, 0x75, 0x74,
    0x0a, 0x20, 0x2a, 0x20, 0x57, 0x49, 0x54, 0x48,
    0x4f, 0x55, 0x54, 0x20, 0x41, 0x4e, 0x59, 0x20,
    0x57, 0x41, 0x52, 0x52, 0x41, 0x4e, 0x54, 0x59,
    0x3b, 0x20, 0x77, 0x69, 0x74, 0x68, 0x6f, 0x75,
    0x74, 0x20, 0x65, 0x76, 0x65, 0x6e, 0x20, 0x74,
    0x68, 0x65, 0x20, 0x69, 0x6d, 0x70, 0x6c, 0x69,
    0x65, 0x64, 0x20, 0x77, 0x61, 0x72, 0x72, 0x61,
    0x6e, 0x74, 0x79, 0x20, 0x6f, 0x66, 0x0a, 0x20,
    0x2a, 0x20, 0x4d, 0x45, 0x52, 0x43, 0x48, 0x41,
    0x4e, 0x54, 0x41, 0x42, 0x49, 0x4c, 0x49, 0x54,
    0x59, 0x20, 0x6f, 0x72, 0x20, 0x46, 0x49, 0x54,
    0x4e, 0x45, 0x53, 0x53, 0x20, 0x46, 0x4f, 0x52,
    0x20, 0x41, 0x20, 0x50, 0x41, 0x52, 0x54, 0x49,
    0x43, 0x55, 0x4c, 0x41, 0x52, 0x20, 0x50, 0x55,
    0x52, 0x50, 0x4f, 0x53, 0x45, 0x2e, 0x20, 0x20,
    0x53, 0x65, 0x65, 0x20, 0x74, 0x68, 0x65, 0x20,
    0x47, 0x4e, 0x55, 0x0a, 0x20, 0x2a, 0x20, 0x47,
    0x65, 0x6e, 0x65, 0x72, 0x61, 0x6c, 0x20, 0x50,
    0x75, 0x62, 0x6c, 0x69, 0x63, 0x20, 0x4c, 0x69,
    0x63, 0x65, 0x6e, 0x73, 0x65, 0x20, 0x66, 0x6f,
    0x72, 0x20, 0x6d, 0x6f, 0x72, 0x65, 0x20, 0x64,
    0x65, 0x74, 0x61, 0x69, 0x6c, 0x73, 0x2e, 0x0a,
    0x20, 0x2a, 0x0a, 0x20, 0x2a, 0x20, 0x59, 0x6f,
    0x75, 0x20, 0x73, 0x68, 0x6f, 0x75, 0x6c, 0x64,
    0x20, 0x68, 0x61, 0x76, 0x65, 0x20, 0x72, 0x65,
    0x63, 0x65, 0x69, 0x76, 0x65, 0x64, 0x20, 0x61,
    0x20, 0x63, 0x6f, 0x70, 0x79, 0x20, 0x6f, 0x66,
    0x20, 0x74, 0x68, 0x65, 0x20, 0x47, 0x4e, 0x55,
    0x20, 0x47, 0x65, 0x6e, 0x65, 0x72, 0x61, 0x6c,
    0x20, 0x50, 0x75, 0x62, 0x6c, 0x69, 0x63, 0x20,
    0x4c, 0x69, 0x63, 0x65, 0x6e, 0x73, 0x65, 0x0a,
    0x20, 0x2a, 0x20, 0x61, 0x6c, 0x6f, 0x6e, 0x67,
    0x20, 0x77, 0x69, 0x74, 0x68, 0x20, 0x74, 0x68,
    0x69, 0x73, 0x20, 0x70, 0x72, 0x6f, 0x67, 0x72,
    0x61, 0x6d, 0x3b, 0x20, 0x73, 0x65, 0x65, 0x20,
    0x74, 0x68, 0x65, 0x20, 0x66, 0x69, 0x6c, 0x65,
    0x20, 0x43, 0x4f, 0x50, 0x59, 0x49, 0x4e, 0x47,
    0x2e, 0x20, 0x20, 0x49, 0x66, 0x20, 0x6e, 0x6f,
    0x74, 0x2c, 0x20, 0x77, 0x72, 0x69, 0x74, 0x65,
    0x20, 0x74, 0x6f, 0x0a, 0x20, 0x2a, 0x20, 0x74,
    0x68, 0x65, 0x20, 0x46, 0x72, 0x65, 0x65, 0x20,
    0x53, 0x6f, 0x66, 0x74, 0x77, 0x61, 0x72, 0x65,
    0x20, 0x46, 0x6f, 0x75, 0x6e, 0x64, 0x61, 0x74,
    0x69, 0x6f, 0x6e, 0x2c, 0x20, 0x35, 0x31, 0x20,
    0x46, 0x72, 0x61, 0x6e, 0x6b, 0x6c, 0x69, 0x6e,
    0x20, 0x53, 0x74, 0x72, 0x65, 0x65, 0x74, 0x2c,
    0x20, 0x46, 0x69, 0x66, 0x74, 0x68, 0x20, 0x46,
    0x6c, 0x6f, 0x6f, 0x72, 0x2c, 0x0a, 0x20, 0x2a,
    0x20, 0x42, 0x6f, 0x73, 0x74, 0x6f, 0x6e, 0x2c,
    0x20, 0x4d, 0x41, 0x20, 0x30, 0x32, 0x31, 0x31,
    0x30, 0x2d, 0x31, 0x33, 0x30, 0x31, 0x2c, 0x20,
    0x55, 0x53, 0x41, 0x2e, 0x0a, 0x20, 0x2a, 0x2f,
    0x0a,
};

static object_data some = {
    some_bytes,
    sizeof(some_bytes),
    "fd8430bc864cfcd5f10e5590f8a447e01b942bfe",
    "blob",
    "test-objects/fd",
    "test-objects/fd/8430bc864cfcd5f10e5590f8a447e01b942bfe",
    some_data,
    sizeof(some_data),
};

BEGIN_TEST(read_loose_commit)
    git_odb *db;
    git_oid id;
    git_rawobj obj;

    must_pass(write_object_files(odb_dir, &commit));
    must_pass(git_odb_open(&db, odb_dir));
    must_pass(git_oid_mkstr(&id, commit.id));

    must_pass(git_odb_read(&obj, db, &id));
    must_pass(cmp_objects(&obj, &commit));

    git_rawobj_close(&obj);
    git_odb_close(db);
    must_pass(remove_object_files(odb_dir, &commit));
END_TEST

BEGIN_TEST(read_loose_tree)
    git_odb *db;
    git_oid id;
    git_rawobj obj;

    must_pass(write_object_files(odb_dir, &tree));
    must_pass(git_odb_open(&db, odb_dir));
    must_pass(git_oid_mkstr(&id, tree.id));

    must_pass(git_odb_read(&obj, db, &id));
    must_pass(cmp_objects(&obj, &tree));

    git_rawobj_close(&obj);
    git_odb_close(db);
    must_pass(remove_object_files(odb_dir, &tree));
END_TEST

BEGIN_TEST(read_loose_tag)
    git_odb *db;
    git_oid id;
    git_rawobj obj;

    must_pass(write_object_files(odb_dir, &tag));
    must_pass(git_odb_open(&db, odb_dir));
    must_pass(git_oid_mkstr(&id, tag.id));

    must_pass(git_odb_read(&obj, db, &id));
    must_pass(cmp_objects(&obj, &tag));

    git_rawobj_close(&obj);
    git_odb_close(db);
    must_pass(remove_object_files(odb_dir, &tag));
END_TEST

BEGIN_TEST(read_loose_zero)
    git_odb *db;
    git_oid id;
    git_rawobj obj;

    must_pass(write_object_files(odb_dir, &zero));
    must_pass(git_odb_open(&db, odb_dir));
    must_pass(git_oid_mkstr(&id, zero.id));

    must_pass(git_odb_read(&obj, db, &id));
    must_pass(cmp_objects(&obj, &zero));

    git_rawobj_close(&obj);
    git_odb_close(db);
    must_pass(remove_object_files(odb_dir, &zero));
END_TEST

BEGIN_TEST(read_loose_one)
    git_odb *db;
    git_oid id;
    git_rawobj obj;

    must_pass(write_object_files(odb_dir, &one));
    must_pass(git_odb_open(&db, odb_dir));
    must_pass(git_oid_mkstr(&id, one.id));

    must_pass(git_odb_read(&obj, db, &id));
    must_pass(cmp_objects(&obj, &one));

    git_rawobj_close(&obj);
    git_odb_close(db);
    must_pass(remove_object_files(odb_dir, &one));
END_TEST

BEGIN_TEST(read_loose_two)
    git_odb *db;
    git_oid id;
    git_rawobj obj;

    must_pass(write_object_files(odb_dir, &two));
    must_pass(git_odb_open(&db, odb_dir));
    must_pass(git_oid_mkstr(&id, two.id));

    must_pass(git_odb_read(&obj, db, &id));
    must_pass(cmp_objects(&obj, &two));

    git_rawobj_close(&obj);
    git_odb_close(db);
    must_pass(remove_object_files(odb_dir, &two));
END_TEST

BEGIN_TEST(read_loose_some)
    git_odb *db;
    git_oid id;
    git_rawobj obj;

    must_pass(write_object_files(odb_dir, &some));
    must_pass(git_odb_open(&db, odb_dir));
    must_pass(git_oid_mkstr(&id, some.id));

    must_pass(git_odb_read(&obj, db, &id));
    must_pass(cmp_objects(&obj, &some));

    git_rawobj_close(&obj);
    git_odb_close(db);
    must_pass(remove_object_files(odb_dir, &some));
END_TEST

