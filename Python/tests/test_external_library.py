from pathlib import Path
from external_library import ExternalLibraryObject


filename = str(Path(__file__).parent.parent.parent / 'ExternalLibrary' / 'Resources' / 'Data' / 'data.txt')


def test_external_function():
    from external_library import external_library_function
    y = external_library_function(filename, (1, 2))
    assert y == (4, 5)


def test_external_object():
    external_object = ExternalLibraryObject(filename)
    y = external_object.evaluate((1, 2))
    assert y == (4, 5)