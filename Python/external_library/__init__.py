
def external_library_function(filename, u):

    with open(filename, 'r') as file:
        k = float(file.readline())

    return tuple(v + k for v in u)


class ExternalLibraryObject(object):

    def __init__(self, filename):
        with open(filename, 'r') as file:
            self.k = float(file.readline())

    def evaluate(self, u):
        return tuple(v + self.k for v in u)
