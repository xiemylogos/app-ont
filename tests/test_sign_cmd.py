
import pytest

from ragger.backend.interface import BackendInterface
from ragger.error import ExceptionRAPDU
from ragger.firmware import Firmware
from ragger.navigator import Navigator, NavInsID
from ragger.navigator.navigation_scenario import NavigateWithScenario

from application_client.boilerplate_transaction import Transaction
from application_client.boilerplate_command_sender import BoilerplateCommandSender, Errors
from application_client.boilerplate_response_unpacker import unpack_get_public_key_response, unpack_sign_tx_response
from utils import check_signature_validity
import hashlib
from utils import hex_to_bytes

# In this tests we check the behavior of the device when asked to sign a transaction
import logging

# Configure logger
logger = logging.getLogger("test_logger")
logger.setLevel(logging.DEBUG)

# In this test we send to the device a transaction to sign and validate it on screen
# The transaction is short and will be sent in one chunk
# We will ensure that the displayed information is correct by using screenshots comparison
def test_sign_tx_short_tx(backend, scenario_navigator):
    # Use the app interface instead of raw interface
    client = BoilerplateCommandSender(backend)
    # The path used for this entire test
    path: str = "m/44'/1024'/0'/0/0"

    # First we need to get the public key of the device in order to build the transaction
    rapdu = client.get_public_key(path=path)
    logger.debug("rapu.data.hex:%s",rapdu.data.hex())

    _, public_key, _, _ = unpack_get_public_key_response(rapdu.data)
    logger.debug("pubkey:%s",public_key.hex())

    # Create the transaction that will be sent to the device for signing
    transaction = Transaction(
        #00d1f98f87a4c409000000000000204e0000000000009b50e5a049679c32e4660266f8814001bde3e6fc7100c66b149b50e5a049679c32e4660266f8814001bde3e6fc6a7cc814f65049e55b9d17bb039d02de7bb9a630a3196cfb6a7cc8516a7cc86c51c1087472616e736665721400000000000000000000000000000000000000010068164f6e746f6c6f67792e4e61746976652e496e766f6b6500
        rawtx = "00d1af112998c409000000000000204e000000000000ae6393b337e4a8d856ec00d75af134fdcd6bfe297e00c66b14ae6393b337e4a8d856ec00d75af134fdcd6bfe296a7cc814e7f1d24e2d3bb7fa3051c732507ee74753756ff36a7cc80b3a6fb192f2db24f1131a016a7cc86c51c10a7472616e7366657256321400000000000000000000000000000000000000020068164f6e746f6c6f67792e4e61746976652e496e766f6b6500"
        #rawtx= "00d1ccdcfce3c409000000000000204e000000000000c457070c4f6e527d411271c4a26786a07f9939e37e00c66b14c457070c4f6e527d411271c4a26786a07f9939e36a7cc814165d30185b2940ace08ee685c5e6bc1af11dd8606a7cc80b690730780e405b377f6d096a7cc86c51c10a7472616e7366657256321400000000000000000000000000000000000000020068164f6e746f6c6f67792e4e61746976652e496e766f6b6500"
        #rawtx= "00d15a76b324c409000000000000204e000000000000b59436f0ed2617bd09ae460fca4b5cfc8f995db58300c66b14b59436f0ed2617bd09ae460fca4b5cfc8f995db56a7cc814b59436f0ed2617bd09ae460fca4b5cfc8f995db56a7cc810000064a7b3b6e00d00000000000000006a7cc86c51c10a7472616e7366657256321400000000000000000000000000000000000000020068164f6e746f6c6f67792e4e61746976652e496e766f6b6500"
        #rawtx= "00d1beb7a948c409000000000000204e000000000000c457070c4f6e527d411271c4a26786a07f9939e37300c66b14c457070c4f6e527d411271c4a26786a07f9939e36a7cc8149b50e5a049679c32e4660266f8814001bde3e6fc6a7cc8516a7cc86c51c10a7472616e7366657256321400000000000000000000000000000000000000020068164f6e746f6c6f67792e4e61746976652e496e766f6b6500"
        #rawtx = "00d17df01b61c409000000000000204e000000000000c457070c4f6e527d411271c4a26786a07f9939e37b00c66b14c457070c4f6e527d411271c4a26786a07f9939e36a7cc8149b50e5a049679c32e4660266f8814001bde3e6fc6a7cc808000064a7b3b6e00d6a7cc86c51c10a7472616e7366657256321400000000000000000000000000000000000000020068164f6e746f6c6f67792e4e61746976652e496e766f6b6500"
    ).serialize()

    # Send the sign device instruction.
    # As it requires on-screen validation, the function is asynchronous.
    # It will yield the result when the navigation is done
    logger.debug("tx:%s",transaction.hex())
    with client.sign_tx(path=path, transaction=transaction):
        # Validate the on-screen request by performing the navigation appropriate for this device
        scenario_navigator.review_approve()

    # The device as yielded the result, parse it and ensure that the signature is correct
    response = client.get_async_response().data
    _, der_sig, _ = unpack_sign_tx_response(response)
    first_hash = hashlib.sha256(transaction).digest()
    second_hash = hashlib.sha256(first_hash).digest()
    logger.debug("pubkey:%s,der_sig:%s,transaction:%s,first_hash:%s",public_key.hex(),der_sig.hex(),transaction.hex(),first_hash.hex())
    assert check_signature_validity(public_key, der_sig, second_hash)


# Transaction signature refused test
# The test will ask for a transaction signature that will be refused on screen
def test_sign_tx_refused(backend, scenario_navigator):
    # Use the app interface instead of raw interface
    client = BoilerplateCommandSender(backend)
    path: str = "m/44'/1024'/0'/0/0"

    rapdu = client.get_public_key(path=path)
    _, pub_key, _, _ = unpack_get_public_key_response(rapdu.data)

    transaction = Transaction(
        rawtx = "00d115ae02abc409000000000000204e00000000000005815d34e0e9ab73a175ec86ffb24aad5bee20f17b00c66b1405815d34e0e9ab73a175ec86ffb24aad5bee20f16a7cc8141451108489337c8055a9c1ed9158c947d22070d76a7cc808000064a7b3b6e00d6a7cc86c51c10a7472616e7366657256321400000000000000000000000000000000000000020068164f6e746f6c6f67792e4e61746976652e496e766f6b6500"
    ).serialize()

    with pytest.raises(ExceptionRAPDU) as e:
        with client.sign_tx(path=path, transaction=transaction):
            scenario_navigator.review_reject()

    # Assert that we have received a refusal
    assert e.value.status == Errors.SW_DENY
    assert len(e.value.data) == 0
