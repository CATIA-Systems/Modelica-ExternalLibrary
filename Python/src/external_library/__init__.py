def external_library_function(filename: str, u: [float]) -> [float]:

    with open(filename, 'r') as file:
        k = float(file.readline())

    return tuple(v + k for v in u)


class ExternalLibraryObject(object):

    def __init__(self, filename: str):
        with open(filename, 'r') as file:
            self.k = float(file.readline())

    def evaluate(self, u: [float]) -> [float]:
        return tuple(v + self.k for v in u)
