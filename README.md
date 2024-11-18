# XAH Wallet App for Ledger devices

[![Ensure compliance with Ledger guidelines](https://github.com/LedgerHQ/app-xah/actions/workflows/guidelines_enforcer.yml/badge.svg)](https://github.com/LedgerHQ/app-xah/actions/workflows/guidelines_enforcer.yml)

[![Build and run functional tests using ragger through reusable workflow](https://github.com/LedgerHQ/app-xah/actions/workflows/build_and_functional_tests.yml/badge.svg)](https://github.com/LedgerHQ/app-xah/actions/workflows/build_and_functional_tests.yml)

## Introduction

This repository contains the source code for the XAH wallet app
that makes it possible to securely store XAH and assets issued
on the XAH Ledger using Ledger Nano devices.

To add Ledger Nano S and Ledger Nano X support in your application, please see the
NPM package [hw-app-xah](https://www.npmjs.com/package/@ledgerhq/hw-app-xah)
and the examples below.

## Features

The XAH wallet app comes with the following features:

- Support for all transaction types:
  - AccountSet
  - AccountDelete
  - CheckCancel
  - CheckCash
  - CheckCreate
  - ClaimReward
  - DepositPreauth
  - EscrowCancel
  - EscrowCreate
  - EscrowFinish
  - Invoke
  - Import
  - OfferCancel
  - OfferCreate
  - Payment
  - PaymentChannelClaim
  - PaymentChannelCreate
  - PaymentChannelFund
  - SetHook
  - SetRegularKey
  - SignerListSet
  - TrustSet
  - URITokenBurn
  - URITokenBuy
  - URITokenCreateSellOffer
  - URITokenCancelSellOffer
  - URITokenMint
  - Remit
- Support for all transaction common fields such as memos, hook parameters, network id
- Support for issued assets such as SOLO, stocks and ETFs
- Support for signing on behalf of others
- Support for multi-signing
- Unified UI across Ledger Nano devices

## User Interface

The user interface primarily consists of the idle menu and the transaction
review menu.

### Idle Menu

Upon starting the app on your device you are immediately greeted by
the idle menu. This menu, as the name implies, is used when the device is in
its idle state. At this point, an external application may initiate a
transaction, which opens up the review menu.

![Idle menu](img/idle-menu.png)

### Review Menu

When reviewing a transaction the entire UI is dedicated to displaying the
transaction details. You can page through all the details by using the left
and right buttons on your device, as indicated by arrows on the screen.

![Review menu](img/review-menu.png)

Fields in arrays are suffixed with their array index in square brackets. See example below.

![Array field in review menu](img/review-array.png)

PathSet fields have their index information shown in square brackets on the
form `[Pi: Sj]`, where `i` is the path index and `j` is the step index within
that path. See example below.

![Path field in review menu](img/review-path.png)

In order to take action on the transaction, you must first page through and review all transaction details.
The last two items in the review menu are 'Sign transaction' and 'Reject'.

![Approval menu](img/approval-menu.png)

Page to either 'Sign transaction' or 'Reject' and press both buttons simultaneously to confirm your action.

## Usage

In order to initiate transactions from NodeJS or a browser client, the library
[hw-app-xah](https://www.npmjs.com/package/@ledgerhq/hw-app-xah) can be used.

An example of a basic payment transaction using this library is shown below:

```javascript
import Transport from "@ledgerhq/hw-transport-node-hid";
// import Transport from "@ledgerhq/hw-transport-u2f"; // for browser
import Xah from "@ledgerhq/hw-app-xah";
import { encode } from "ripple-binary-codec";

function establishConnection() {
  return Transport.create().then((transport) => new Xah(transport));
}

function fetchAddress(xah) {
  return xah.getAddress("44'/144'/0'/0/0").then((deviceData) => {
    return {
      xah,
      address: deviceData.address,
      publicKey: deviceData.publicKey.toUpperCase(),
    };
  });
}

function signTransaction(context, transaction) {
  const preparedTransaction = {
    Account: context.address,
    SigningPubKey: context.publicKey,
    ...transaction,
  };

  const transactionBlob = encode(preparedTransaction);

  console.log("Sending transaction to device for approval...");
  return context.xah.signTransaction("44'/144'/0'/0/0", transactionBlob);
}

const transactionJSON = {
  TransactionType: "Payment",
  Destination: "rTooLkitCksh5mQa67eaa2JaWHDBnHkpy",
  Amount: "1000000",
  Fee: "15",
  Flags: 2147483648,
  Sequence: 57,
};

establishConnection()
  .then((xah) => fetchAddress(xah))
  .then((context) => signTransaction(context, transactionJSON))
  .then((signature) => console.log(`Signature: ${signature}`))
  .catch((e) => console.log(`An error occurred (${e.message})`));
```

### Advanced Usage

#### Multi-signing a Transaction

It is also possible to perform parallel multi-signing using the XAH wallet
app. This is done by sourcing a list of signatures for the transaction
and appending them to the `Signers` field of the transaction before submitting
it for processing. An example of combining a couple of externally sourced signatures
with a signature of the Ledger device is shown below (uses imports and functions declared in previous example).

```javascript
const transactionJSON = {
  Account: "r4PCuDkjuV2e23xVP8ChkVxo1aG2Ufpkjb",
  TransactionType: "Payment",
  Destination: "rTooLkitCksh5mQa67eaa2JaWHDBnHkpy",
  Amount: "1000000",
  Fee: "15",
  Flags: 2147483648,
  Sequence: 47,
  SigningPubKey: "", // Must be blank
};

// Sourced externally from other signing parties, replace "..." with actual values.
const otherSigners = [
  {
    Signer: {
      Account: "...",
      SigningPubKey: "...",
      TxnSignature: "...",
    },
  },
  {
    Signer: {
      Account: "...",
      SigningPubKey: "...",
      TxnSignature: "...",
    },
  },
];

function retrieveSignerData(transaction) {
  return establishConnection()
    .then((xah) => fetchAddress(xah))
    .then((context) => {
      return signTransaction(context, transaction).then((signature) => {
        return {
          Signer: {
            Account: context.account,
            SigningPubKey: context.publicKey,
            TxnSignature: signature.toUpperCase(),
          },
        };
      });
    })
    .catch((e) => console.log(`An error occurred (${e.message})`));
}

retrieveSignerData(transactionJSON)
  .then((signer) => {
    return {
      ...transactionJSON,
      Signers: [...otherSigners, signer],
    };
  })
  .then((transaction) => console.log(transaction))
  .catch((e) => console.log(`An error occurred (${e.message})`));
```

### Limitations

Because of resource constraints the following limits apply for the respective
hardware wallet:

#### Ledger Nano S

- Maximum fields per transaction: 24 fields
- Maximum displayed field value length: 128 characters
- Maximum transaction size: 800 bytes
- Maximum number of elements per array field: 8 elements
- Multi-sign support: Parallel only

#### Ledger Nano X

- Maximum fields per transaction: 60 fields
- Maximum displayed field value length: 1024 characters
- Maximum transaction size: 10 000 bytes
- Maximum number of elements per array field: 8 elements
- Multi-sign support: Parallel only

## Building

Make sure that you have configured a development environment as outlined in [the development
documentation](https://developers.ledger.com/docs/device-app/getting-started)
for Ledger devices. Then run make from the repository root to build the app:

```sh
make
```

## Installing

To upload the app to your device, run the following command:

```sh
make load
```

## Testing

Manual testing can be conducted with the help of the testing utility
[Xahau/ledger-tests-xah](https://github.com/Xahau/ledger-tests-xah).
Make sure that your device is running the latest firmware and then follow
the instructions in the test repository.
