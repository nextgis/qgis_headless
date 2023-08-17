def test_httpbin(fetch):
    binary = fetch("http://httpbin.org/bytes/64", mode="binary")
    assert isinstance(binary, bytes)
    assert len(binary) == 64
