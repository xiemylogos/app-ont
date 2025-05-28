from io import BytesIO
from typing import Union

class PersonalMsgError(Exception):
    pass


class PersonalMsg:
    def __init__(self,
                 personalmsg: str) -> None:
        self.personalmsg: bytes = personalmsg.encode('utf-8')


    def serialize(self) -> bytes:
        return b"".join([
            self.personalmsg
        ])

    @classmethod
    def from_bytes(cls, hexa: Union[bytes, BytesIO]):
        buf: BytesIO = BytesIO(hexa) if isinstance(hexa, bytes) else hexa
        personalmsg: str = buf.read().decode('utf-8')  # Use buf.read() directly
        return cls(personalmsg=personalmsg)
