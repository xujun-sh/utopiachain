// Copyright (c) 2010 Satoshi Nakamoto
// Copyright (c) 2009-2017 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <chainparams.h>
#include <consensus/merkle.h>

#include <tinyformat.h>
#include <util.h>
#include <utilstrencodings.h>

#include <assert.h>

#include <chainparamsseeds.h>

static CBlock CreateGenesisBlock(const char* pszTimestamp, const CScript& genesisOutputScript, uint32_t nTime, uint32_t nNonce, uint32_t nBits, int32_t nVersion, const CAmount& genesisReward)
{
    CMutableTransaction txNew;
    txNew.nVersion = 1;
    txNew.vin.resize(1);
    txNew.vout.resize(1);
    //txNew.vin[0].scriptSig = CScript() << 545259519 << CScriptNum(4) << std::vector<unsigned char>((const unsigned char*)pszTimestamp, (const unsigned char*)pszTimestamp + strlen(pszTimestamp));
    txNew.vin[0].scriptSig = CScript() << 486604799 << CScriptNum(4) << std::vector<unsigned char>((const unsigned char*)pszTimestamp, (const unsigned char*)pszTimestamp + strlen(pszTimestamp));		
    txNew.vout[0].nValue = genesisReward;
    txNew.vout[0].scriptPubKey = genesisOutputScript;

    CBlock genesis;
    genesis.nTime    = nTime;
    genesis.nBits    = nBits;
    genesis.nNonce   = nNonce;
    genesis.nVersion = nVersion;
    genesis.vtx.push_back(MakeTransactionRef(std::move(txNew)));
    genesis.hashPrevBlock.SetNull();
    genesis.hashMerkleRoot = BlockMerkleRoot(genesis);
    return genesis;
}

/**
 * Build the genesis block. Note that the output of its generation
 * transaction cannot be spent since it did not originally exist in the
 * database.
 *
 * CBlock(hash=000000000019d6, ver=1, hashPrevBlock=00000000000000, hashMerkleRoot=4a5e1e, nTime=1231006505, nBits=1d00ffff, nNonce=2083236893, vtx=1)
 *   CTransaction(hash=4a5e1e, ver=1, vin.size=1, vout.size=1, nLockTime=0)
 *     CTxIn(COutPoint(000000, -1), coinbase 04ffff001d0104455468652054696d65732030332f4a616e2f32303039204368616e63656c6c6f72206f6e206272696e6b206f66207365636f6e64206261696c6f757420666f722062616e6b73)
 *     CTxOut(nValue=50.00000000, scriptPubKey=0x5F1DF16B2B704C8A578D0B)
 *   vMerkleTree: 4a5e1e
 */
static CBlock CreateGenesisBlock(uint32_t nTime, uint32_t nNonce, uint32_t nBits, int32_t nVersion, const CAmount& genesisReward)
{   
    //const char* pszTimestamp = "EverMarket: Trading for Whatever, with Whoever, from Wherever, at Whenever, by However, & no Feels! 04/09/2018";       //--//
    //const CScript genesisOutputScript = CScript() << ParseHex("042db5eccd3100e0373bf51daaa0857eb1410bd8772c8c8513599f15c3f5a0decaf6bfedb4f25e454328f67f6a5823f3f7c7efda274a276a37189139957402435d") << OP_CHECKSIG; //--//

    //const char* pszTimestamp = "EverMarket: Trading for Whatever, with Whoever no Fee! 20180708";
    const char* pszTimestamp = "To trade:whatever,whoever,wherever,whenever,however,no fee! 2018/7/8";
    const CScript genesisOutputScript = CScript() << ParseHex("042db5eccd3100e0373bf51daaa0857eb1410bd8772c8c8513599f15c3f5a0decaf6bfedb4f25e454328f67f6a5823f3f7c7efda274a276a37189139957402435d") << OP_CHECKSIG;

    return CreateGenesisBlock(pszTimestamp, genesisOutputScript, nTime, nNonce, nBits, nVersion, genesisReward);
}

void CChainParams::UpdateVersionBitsParameters(Consensus::DeploymentPos d, int64_t nStartTime, int64_t nTimeout)
{
    consensus.vDeployments[d].nStartTime = nStartTime;
    consensus.vDeployments[d].nTimeout = nTimeout;
}

/**
 * Main network
 */
/**
 * What makes a good checkpoint block?
 * + Is surrounded by blocks with reasonable timestamps
 *   (no blocks before with a timestamp after, none after with
 *    timestamp before)
 * + Contains no strange transactions
 */

class CMainParams : public CChainParams {
public:
    CMainParams() {
        strNetworkID = "main";
        consensus.nSubsidyHalvingInterval = 210000;
        //consensus.BIP16Height = 0;    //--// always BIP16   00000000000000ce80a7e057163a4db1d5ad7b20fb6f598c9597b9665c8fb0d4 - April 1, 2012
        //consensus.BIP34Height = 0;    //--// always BIP34
        //consensus.BIP34Hash = uint256S("0x000000000000024b89b42a942fe0d9fea3bb44ab7bd1b19115dd6a759c0808b8");
        //consensus.BIP65Height = 0;    //--// always BIP65  000000000000000004c2b624ed5d7756c508d90fd0da2c7c679febfa6c4735f0
        //consensus.BIP66Height = 0;    //--// always BIP66  00000000000000000379eaa19dce8c9b722d46ae6a57c2f1a988119488b50931
        consensus.powLimit = uint256S("00000000ffffffffffffffffffffffffffffffffffffffffffffffffffffffff");            
                                        
        consensus.nPowTargetTimespan = 14 * 24 * 60 * 60; // two weeks  
        consensus.nPowTargetSpacing = 10 * 60;
        consensus.fPowAllowMinDifficultyBlocks = false;
        consensus.fPowNoRetargeting = false;
        consensus.nRuleChangeActivationThreshold = 1916; // 95% of 2016
        consensus.nMinerConfirmationWindow = 2016; // nPowTargetTimespan / nPowTargetSpacing
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = 1199145601; // January 1, 2008
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = 1230767999; // December 31, 2008

        // Deployment of BIP68, BIP112, and BIP113.
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].bit = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nStartTime = 1462060800; // May 1st, 2016
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nTimeout = 1493596800; // May 1st, 2017

         // Deployment of SegWit (BIP141, BIP143, and BIP147)
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].bit = 1;
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nStartTime = 1479168000; // November 15th, 2016.
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nTimeout = 1510704000; // November 15th, 2017.

        // The best chain should have at least this much work.
        consensus.nMinimumChainWork = uint256S("0x00");  //--//

        // By default assume that the signatures in ancestors of this block are valid.
        consensus.defaultAssumeValid = uint256S("0x00"); //--//

        /**
         * The message start string is designed to be unlikely to occur in normal data.
         * The characters are rarely used upper ASCII, not valid as UTF-8, and produce
         * a large 32-bit integer with any alignment.
         */
        pchMessageStart[0] = 0xa2;   //--//
        pchMessageStart[1] = 0xb4;   //--//
        pchMessageStart[2] = 0xc6;   //--//
        pchMessageStart[3] = 0xd8;   //--//
	/*	
	pchMessageStart[0] = 0x2a;   //--//
        pchMessageStart[1] = 0x4b;   //--//
        pchMessageStart[2] = 0x6c;   //--//
        pchMessageStart[3] = 0x8d;   //--//
	*/
        nDefaultPort = 9567;  //--//   
        nPruneAfterHeight = 100000; 
/*

./generator 042db5eccd3100e0373bf51daaa0857eb1410bd8772c8c8513599f15c3f5a0decaf6bfedb4f25e454328f67f6a5823f3f7c7efda274a276a37189139957402435d "To trade:whatever,whoever,wherever,whenever,however,no fee! 2018/7/8" 486604799

Coinbase: 04ffff001d010443457665724d61726b65743a2054726164696e6720666f722057686174657665722c20776974682057686f65766572206e6f204665656c73212030342f30392f32303138

PubkeyScript: 41042db5eccd3100e0373bf51daaa0857eb1410bd8772c8c8513599f15c3f5a0decaf6bfedb4f25e454328f67f6a5823f3f7c7efda274a276a37189139957402435dac

Merkle Hash: 364ecc1861373c3c2c67dd65ec8e795b6435b19761e95c170f2332a8744bd176
Byteswapped: 76d14b74a832230f175ce96197b135645b798eec65dd672c3c3c376118cc4e36
Generating block...
321111 Hashes/s, Nonce 30117587647
Block found!
Hash: 00000000d9540efed6c7b96b6d68952d331f968d8dcfbdff84ccd10393f0aeaf
Nonce: 3012042094
Unix time: 1531049810

./generator 042db5eccd3100e0373bf51daaa0857eb1410bd8772c8c8513599f15c3f5a0decaf6bfedb4f25e454328f67f6a5823f3f7c7efda274a276a37189139957402435d "To trade:whatever,whoever,wherever,whenever,however,no fee! 2018/7/8" 486604799

Coinbase: 04ffff001d010444546f2074726164653a77686174657665722c77686f657665722c77686572657665722c7768656e657665722c686f77657665722c6e6f206665652120323031382f372f38

PubkeyScript: 41042db5eccd3100e0373bf51daaa0857eb1410bd8772c8c8513599f15c3f5a0decaf6bfedb4f25e454328f67f6a5823f3f7c7efda274a276a37189139957402435dac

Merkle Hash: b88540453187252090bedbf2849106c0713791ac32962c71372817bfbc02bcf1
Byteswapped: f1bc02bcbf172837712c9632ac913771c0069184f2dbbe9020258731454085b8
Generating block...
1933337 Hashes/s, Nonce 654964924
Block found!
Hash: 00000000d6a60a86bcea7bfa28012c368e7c19489beb9b3feba28903ae1c7f0d
Nonce: 656505664
Unix time: 1537013470
*/

        //genesis = CreateGenesisBlock(1531049810, 3012042094, 0x207fffff, 1, 21000000 * COIN);      //--// all 21,000,000 coins
        ////genesis = CreateGenesisBlock(1523410697, 285676872, 0x1d00ffff, 1, 21000000 * COIN);
        //consensus.hashGenesisBlock = genesis.GetHash();
        //assert(consensus.hashGenesisBlock == uint256S("0x000000009754350b26c9d74e622f0dbb7c4de794325776dd1b8bada3f77f38de"));     //--// need to put in new hash of genesis block
        //assert(genesis.hashMerkleRoot == uint256S("0x3ca2c4d58adba82b577d0b0a25a4a3440c58dd358f7d0db00f93ef3af5d30114"));         //--// need to put in new hash of genesis block merkle root

	//genesis = CreateGenesisBlock(1523652107, 2761769460, 0x207fffff, 1, 21000000 * COIN);
	genesis = CreateGenesisBlock(1537013470, 656505664, 0x1d00ffff, 1, 21000000 * COIN);
        consensus.hashGenesisBlock = genesis.GetHash();
        assert(consensus.hashGenesisBlock == uint256S("0x00000000d6a60a86bcea7bfa28012c368e7c19489beb9b3feba28903ae1c7f0d"));     //--// need to put in new hash of genesis block
        assert(genesis.hashMerkleRoot == uint256S("0xf1bc02bcbf172837712c9632ac913771c0069184f2dbbe9020258731454085b8"));         //--// need to put in new hash of genesis block merkle root

        vFixedSeeds.clear();           //--// doesn't have any fixed seeds
        vSeeds.clear();                //--// doesn't have any DNS seeds
        // Note that of those with the service bits flag, most only support a subset of possible options
       //--//+ vSeeds.emplace_back("169.254.151.189/16", true); //// Yong
       //--//+ vSeeds.emplace_back("dnsseed.bluematt.me", true); // Matt Corallo, only supports x9
       //--// vSeeds.emplace_back("dnsseed.utopiacoin.dashjr.org", false); // Luke Dashjr
       //--// vSeeds.emplace_back("seed.utopiacoinstats.com", true); // Christian Decker, supports x1 - xf
       //--// vSeeds.emplace_back("seed.utopiacoin.jonasschnelli.ch", true); // Jonas Schnelli, only supports x1, x5, x9, and xd
       //--// vSeeds.emplace_back("seed.UTC.petertodd.org", true); // Peter Todd, only supports x1, x5, x9, and xd

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,0);
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,5);
        base58Prefixes[SECRET_KEY] =     std::vector<unsigned char>(1,128);
        base58Prefixes[EXT_PUBLIC_KEY] = {0x04, 0x88, 0xB2, 0x1E};
        base58Prefixes[EXT_SECRET_KEY] = {0x04, 0x88, 0xAD, 0xE4};

        //bech32_hrp = "em";      //--//
	bech32_hrp = "uc";

        //--// vFixedSeeds = std::vector<SeedSpec6>(pnSeed6_main, pnSeed6_main + ARRAYLEN(pnSeed6_main));

        fDefaultConsistencyChecks = false;
        fRequireStandard = true;
        fMineBlocksOnDemand = false;

        checkpointData = {
            {
                { 0, uint256S("0x00000000d6a60a86bcea7bfa28012c368e7c19489beb9b3feba28903ae1c7f0d")},    //--//
    //--//            { 33333, uint256S("0x000000002dd5588a74784eaa7ab0507a18ad16a236e7b1ce69f00d7ddfb5d0a6")},
    //--//            { 74000, uint256S("0x0000000000573993a3c9e41ce34471c079dcf5f52a0e824a81e7f953b8661a20")},
    //--//            {105000, uint256S("0x00000000000291ce28027faea320c8d2b054b2e0fe44a773f3eefb151d6bdc97")},
    //--//            {134444, uint256S("0x00000000000005b12ffd4cd315cd34ffd4a594f430ac814c91184a0d42d2b0fe")},
    //--//            {168000, uint256S("0x000000000000099e61ea72015e79632f216fe6cb33d7899acb35b75c8303b763")},
    //--//            {193000, uint256S("0x000000000000059f452a5f7340de6682a977387c17010ff6e6c3bd83ca8b1317")},
    //--//            {210000, uint256S("0x000000000000048b95347e83192f69cf0366076336c639f9b7228e9ba171342e")},
    //--//            {216116, uint256S("0x00000000000001b4f4b433e81ee46494af945cf96014816a4e2370f11b23df4e")},
    //--//            {225430, uint256S("0x00000000000001c108384350f74090433e7fcf79a606b8e797f065b130575932")},
    //--//            {250000, uint256S("0x000000000000003887df1f29024b06fc2200b55f8af8f35453d7be294df2d214")},
    //--//            {279000, uint256S("0x0000000000000001ae8c72a0b0c301f67e3afca10e819efa9041e458e9bd7e40")},
    //--//            {295000, uint256S("0x00000000000000004d9b4ef50f0f9d686fd69db2e03af35a100370c64632a983")},
            }
        };

        chainTxData = ChainTxData{
            // Data as of block 000000000000000000d97e53664d17967bd4ee50b23abb92e54a34eb222d15ae (height 478913).
            1537013470,        //--// * UNIX timestamp for genesis block (need to put)
            0,        //--// * total number of transactions between genesis and that timestamp
                        //   (the tx=... number in the SetBestChain debug.log lines)
            0        //--// * estimated number of transactions per second after that timestamp
        };
    }
};

/**
 * Testnet (v3)
 */
class CTestNetParams : public CChainParams {
public:
    CTestNetParams() {
        strNetworkID = "test";
        consensus.nSubsidyHalvingInterval = 210000;
        consensus.BIP16Height = 514; // 00000000040b4e986385315e14bee30ad876d8b47f748025b26683116d21aa65
        consensus.BIP34Height = 21111;
        consensus.BIP34Hash = uint256S("0x0000000023b3a96d3484e5abb3755c413e7d41500f8e2a5c3f0dd01299cd8ef8");
        consensus.BIP65Height = 581885; // 00000000007f6655f22f98e72ed80d8b06dc761d5da09df0fa1dc4be4f861eb6
        consensus.BIP66Height = 330776; // 000000002104c8c45e99a8853285a3b592602a3ccde2b832481da85e9e4ba182
        consensus.powLimit = uint256S("0000ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");       //--//
        consensus.nPowTargetTimespan = 14 * 24 * 60 * 60; // two weeks
        consensus.nPowTargetSpacing = 10 * 60;
        consensus.fPowAllowMinDifficultyBlocks = true;
        consensus.fPowNoRetargeting = false;
        consensus.nRuleChangeActivationThreshold = 1512; // 75% for testchains
        consensus.nMinerConfirmationWindow = 2016; // nPowTargetTimespan / nPowTargetSpacing
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = 1199145601; // January 1, 2008
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = 1230767999; // December 31, 2008

        // Deployment of BIP68, BIP112, and BIP113.
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].bit = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nStartTime = 1456790400; // March 1st, 2016
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nTimeout = 1493596800; // May 1st, 2017

        // Deployment of SegWit (BIP141, BIP143, and BIP147)
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].bit = 1;
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nStartTime = 1462060800; // May 1st 2016
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nTimeout = 1493596800; // May 1st 2017

        // The best chain should have at least this much work.
        consensus.nMinimumChainWork = uint256S("0x00");   //--//

        // By default assume that the signatures in ancestors of this block are valid.
        consensus.defaultAssumeValid = uint256S("0x00");  //--//
	/*
        pchMessageStart[0] = 0x1a;                    //--//
        pchMessageStart[1] = 0x3b;                    //--//
        pchMessageStart[2] = 0x5c;                    //--//
        pchMessageStart[3] = 0x7d;                    //--//
	*/
	pchMessageStart[0] = 0x2a;                 //  test net
        pchMessageStart[1] = 0x4b;                //  test net
        pchMessageStart[2] = 0x6c;                //  test net
        pchMessageStart[3] = 0x8d;                //  test net
        
	nDefaultPort = 19567;                         //--//
        nPruneAfterHeight = 1000;

        //genesis = CreateGenesisBlock(1523664586, 3256264841, 0x207fffff, 1, 50 * COIN);  //--// need to change to new time
	genesis = CreateGenesisBlock(1531872954, 2161789762, 0x1d00ffff, 1, 21000000 * COIN);
        consensus.hashGenesisBlock = genesis.GetHash();
        assert(consensus.hashGenesisBlock == uint256S("0x000000000c7faa56e2045ea6c7effcd6df83769fe151561350c9a9238ebac794"));     //--// need to put in new hash of genesis block
        assert(genesis.hashMerkleRoot == uint256S("0xf1bc02bcbf172837712c9632ac913771c0069184f2dbbe9020258731454085b8"));         //--// need to put in new hash of genesis block merkle root


        vFixedSeeds.clear();
        vSeeds.clear();
        // nodes with support for servicebits filtering should be at the top
        //--// vSeeds.emplace_back("testnet-seed.utopiacoin.jonasschnelli.ch", true);
        //--// vSeeds.emplace_back("seed.tUTC.petertodd.org", true);
        //--// vSeeds.emplace_back("seed.testnet.utopiacoin.sprovoost.nl", true);
        //--// vSeeds.emplace_back("testnet-seed.bluematt.me", false);

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,111);
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,196);
        base58Prefixes[SECRET_KEY] =     std::vector<unsigned char>(1,239);
        base58Prefixes[EXT_PUBLIC_KEY] = {0x04, 0x35, 0x87, 0xCF};
        base58Prefixes[EXT_SECRET_KEY] = {0x04, 0x35, 0x83, 0x94};

        //bech32_hrp = "emt";              //--//
	bech32_hrp = "tu";

        //--// vFixedSeeds = std::vector<SeedSpec6>(pnSeed6_test, pnSeed6_test + ARRAYLEN(pnSeed6_test));

        fDefaultConsistencyChecks = false;
        fRequireStandard = false;
        fMineBlocksOnDemand = false;


        checkpointData = {
            {
              	{ 0, uint256S("0x000000000c7faa56e2045ea6c7effcd6df83769fe151561350c9a9238ebac794")},    //--//
            }
        };

        chainTxData = ChainTxData{
            // Data as of block 00000000000001c200b9790dc637d3bb141fe77d155b966ed775b17e109f7c6c (height 1156179)
            1531872954,       //--// 
            0,       //--// 
            0     //--// 
        };

    }
};

/**
 * Regression test
 */
class CRegTestParams : public CChainParams {
public:
    CRegTestParams() {
        strNetworkID = "regtest";
        consensus.nSubsidyHalvingInterval = 150;
        consensus.BIP16Height = 0; // always enforce P2SH BIP16 on regtest
        consensus.BIP34Height = 100000000; // BIP34 has not activated on regtest (far in the future so block v1 are not rejected in tests)
        consensus.BIP34Hash = uint256();
        consensus.BIP65Height = 1351; // BIP65 activated on regtest (Used in rpc activation tests)
        consensus.BIP66Height = 1251; // BIP66 activated on regtest (Used in rpc activation tests)
        consensus.powLimit = uint256S("7fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.nPowTargetTimespan = 14 * 24 * 60 * 60; // two weeks
        consensus.nPowTargetSpacing = 10 * 60;
        consensus.fPowAllowMinDifficultyBlocks = true;
        consensus.fPowNoRetargeting = true;
        consensus.nRuleChangeActivationThreshold = 108; // 75% for testchains
        consensus.nMinerConfirmationWindow = 144; // Faster than normal for regtest (144 instead of 2016)
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = Consensus::BIP9Deployment::NO_TIMEOUT;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].bit = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nStartTime = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nTimeout = Consensus::BIP9Deployment::NO_TIMEOUT;
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].bit = 1;
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nStartTime = Consensus::BIP9Deployment::ALWAYS_ACTIVE;
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nTimeout = Consensus::BIP9Deployment::NO_TIMEOUT;

        // The best chain should have at least this much work.
        consensus.nMinimumChainWork = uint256S("0x00");

        // By default assume that the signatures in ancestors of this block are valid.
        consensus.defaultAssumeValid = uint256S("0x00");
	/*
        pchMessageStart[0] = 0x2a;                //--//
        pchMessageStart[1] = 0x3b;                //--//
        pchMessageStart[2] = 0x4c;                //--//
        pchMessageStart[3] = 0x5d;                //--//
        */
	pchMessageStart[0] = 0xab;               //  regtest net
        pchMessageStart[1] = 0xbc;               //  regtest net
        pchMessageStart[2] = 0xcd;               //  regtest net
        pchMessageStart[3] = 0xde;               //  regtest net

	nDefaultPort = 19678;                     //--//              
        nPruneAfterHeight = 1000;

        //genesis = CreateGenesisBlock(1496688602, 2, 0x207fffff, 1, 50 * COIN);                 //--// need to put in new time
        //consensus.hashGenesisBlock = genesis.GetHash();
        //assert(consensus.hashGenesisBlock == uint256S("0x"));                  //--//  need to put in new hash
        //assert(genesis.hashMerkleRoot == uint256S("0x"));                      //--//  need to put in new hash
/*
./generator 042db5eccd3100e0373bf51daaa0857eb1410bd8772c8c8513599f15c3f5a0decaf6bfedb4f25e454328f67f6a5823f3f7c7efda274a276a37189139957402435d "To trade: whatever, whoever, wherever, whenever, however, no fee! 2018/7/8." 486604799

Coinbase: 04ffff001d010454546f2074726164653a2077686174657665722c2077686f657665722c2077686572657665722c207768656e657665722c20686f77657665722c20776974686f757420616e79206665652120323031382f372f382e

PubkeyScript: 41042db5eccd3100e0373bf51daaa0857eb1410bd8772c8c8513599f15c3f5a0decaf6bfedb4f25e454328f67f6a5823f3f7c7efda274a276a37189139957402435dac

Merkle Hash: 7760b3302bc923d33d2b11d0096142625f56842874206ef052edbdc1dc6c6b37
Byteswapped: 376b6cdcc1bded52f06e20742884565f62426109d0112b3dd323c92b30b36077
Generating block...
286456 Hashes/s, Nonce 25586751563
Block found!
Hash: 000000000d1afaf4444d7c3af50f5b9a06e34b6e0bf5d1aecf5efd513262841e
Nonce: 2558754250
Unix time: 1531256859

*/
	//genesis = CreateGenesisBlock(1523664586, 3256264841, 0x207fffff, 1, 50 * COIN);  //--// need to change to new time
        //genesis = CreateGenesisBlock(1523712589, 305168588, 0x207fffff, 1, 50 * COIN);
	genesis = CreateGenesisBlock(1531872954, 2161789762, 0x1d00ffff, 1, 21000000 * COIN);
        consensus.hashGenesisBlock = genesis.GetHash();
        assert(consensus.hashGenesisBlock == uint256S("0x000000000c7faa56e2045ea6c7effcd6df83769fe151561350c9a9238ebac794"));     //--// need to put in new hash of genesis block
        assert(genesis.hashMerkleRoot == uint256S("0xf1bc02bcbf172837712c9632ac913771c0069184f2dbbe9020258731454085b8"));         //--// need to put in new hash of genesis block merkle root
	

        
        vFixedSeeds.clear(); //!< Regtest mode doesn't have any fixed seeds.
        vSeeds.clear();      //!< Regtest mode doesn't have any DNS seeds.

        fDefaultConsistencyChecks = true;
        fRequireStandard = false;
        fMineBlocksOnDemand = true;

        checkpointData = {
            {
                {0, uint256S("0x000000000c7faa56e2045ea6c7effcd6df83769fe151561350c9a9238ebac794")},  //--//  need new hash
            }
        };

        chainTxData = ChainTxData{
            1531872954,
            0,
            0
        };

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,111);
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,196);
        base58Prefixes[SECRET_KEY] =     std::vector<unsigned char>(1,239);
        base58Prefixes[EXT_PUBLIC_KEY] = {0x04, 0x35, 0x87, 0xCF};
        base58Prefixes[EXT_SECRET_KEY] = {0x04, 0x35, 0x83, 0x94};

        //bech32_hrp = "emrt";                  //--//
	bech32_hrp = "ucrt"; 
    }
};

static std::unique_ptr<CChainParams> globalChainParams;

const CChainParams &Params() {
    assert(globalChainParams);
    return *globalChainParams;
}

std::unique_ptr<CChainParams> CreateChainParams(const std::string& chain)
{
    if (chain == CBaseChainParams::MAIN)
        return std::unique_ptr<CChainParams>(new CMainParams());
    else if (chain == CBaseChainParams::TESTNET)
        return std::unique_ptr<CChainParams>(new CTestNetParams());
    else if (chain == CBaseChainParams::REGTEST)
        return std::unique_ptr<CChainParams>(new CRegTestParams());
    throw std::runtime_error(strprintf("%s: Unknown chain %s.", __func__, chain));
}

void SelectParams(const std::string& network)
{
    SelectBaseParams(network);
    globalChainParams = CreateChainParams(network);
}

void UpdateVersionBitsParameters(Consensus::DeploymentPos d, int64_t nStartTime, int64_t nTimeout)
{
    globalChainParams->UpdateVersionBitsParameters(d, nStartTime, nTimeout);
}
