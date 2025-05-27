#pragma once

#define FROM               "From"
#define TO                 "To"
#define AMOUNT             "Amount"
#define SPENDER            "Spender"
#define GAS_FEE            "Gas Fee"
#define SIGNER             "Signer"
#define CONTRACT_ADDRESS   "Contract Address"
#define PERCENTAGE         "%"
#define NBGL_MSG           "Message"
#ifdef SCREEN_SIZE_WALLET
#define PEER_PUBKEY        "Node Operation Public Key"
#define PEER_INCENTIVE     "Incentive Sharing Ratio (Node)"
#define USER_INCENTIVE     "Incentive Sharing Ratio (User)"
#define NODE_AMOUNT        "Number of Remaining Nodes"
#else
#define PEER_PUBKEY        "Node OP. PK"
#define PEER_INCENTIVE     "Node INCTV. Ratio"
#define USER_INCENTIVE     "User INCTV. Ratio"
#define NODE_AMOUNT        "Remaining Nodes"
#endif
#define WITHDRAW_AMOUNT    "Withdraw Amount"
#define STAKE_AMOUNT       "Stake Amount"
#define UNSTAKE_AMOUNT     "Unstake Amount"
#define TOTAL_PLUS         "Total "
#define STAKE_FEE          "Staking Fee"
#define STAKE_FEE_ONG      "500 ONG"
#define MAX_AUTHORIZE      "Allowed User Stake"
#define STAKE_ADDRESS      "Stake Address"

#define BLIND_SIGN_TX      "Blind Signing Transaction"
#define VERIFY_ONT_ADDRESS "Verify Ontology Address"

#define TRANSFER_TITLE   "Review transaction to send token"
#define TRANSFER_CONTENT "Sign transaction to send token?"

#define TRANSFER_FROM_TITLE   "Review transaction to transfer from others"
#define TRANSFER_FROM_CONTENT "Sign transaction to transfer from others?"

#define APPROVE_TITLE   "Review transaction to approve"
#define APPROVE_CONTENT "Sign transaction to approve?"

#define REGISTER_CANDIDATE_TITLE   "Review transaction to register a node"
#define REGISTER_CANDIDATE_CONTENT "Sign transaction to register a node?"
#define QUIT_NODE_TITLE            "Review transaction to quit the node"
#define QUIT_NODE_CONTENT          "Sign transaction to quit the node?"

#define ADD_INIT_POS_TITLE      "Review transaction to increase node stake"
#define ADD_INIT_POS_CONTENT    "Sign transaction to increase node stake?"
#define REDUCE_INIT_POS_TITLE   "Review transaction to reduce node stake"
#define REDUCE_INIT_POS_CONTENT "Sign transaction to reduce node stake?"

#define AUTHORIZE_FOR_PEER_TITLE      "Review transaction to stake nodes"
#define AUTHORIZE_FOR_PEER_CONTENT    "Sign transaction to stake nodes?"
#define UN_AUTHORIZE_FOR_PEER_TITLE   "Review transaction to unstake nodes"
#define UN_AUTHORIZE_FOR_PEER_CONTENT "Sign transaction to unstake nodes?"

#define CHANGE_MAX_AUTHORIZATION_TITLE   "Review transaction to edit allowed user stake"
#define CHANGE_MAX_AUTHORIZATION_CONTENT "Sign transaction to edit allowed user stake?"
#define SET_FEE_PERCENTAGE_TITLE         "Review transaction to edit incentive sharing ratio"
#define SET_FEE_PERCENTAGE_CONTENT       "Sign transaction to edit incentive sharing ratio?"

#define WITHDRAW_FEE_TITLE   "Review transaction to claim incentives"
#define WITHDRAW_FEE_CONTENT "Sign transaction to claim incentives?"
#define WITHDRAW_TITLE       "Review transaction to withdraw unstaked token"
#define WITHDRAW_CONTENT     "Sign transaction to withdraw unstaked token?"

#define BLIND_SIGNING_TITLE   "Blind sign transaction"
#define BLIND_SIGNING_CONTENT "Accept risk and blind sign transaction?"

#define BLIND_SIGNING      "Transaction cannot be verified!"
#define BLIND_SIGNING_SWITCH_TEXT    "Blind signing"
#define BLIND_SIGNING_SWITCH_SUBTEXT "Enable transaction blind signing"


#define PERSONAL_MSG_TITLE   "Review message"
#define PERSONAL_MSG_CONTENT "Sign message?"
