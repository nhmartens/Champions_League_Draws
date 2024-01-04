
import time
computedProbabilities = {}
fullCompatibilityMatrix = []


seasonLog = {}
seasonId = 0

def initialize(attr1, attr2):
    global seasonId
    attributes = [attr1, attr2]
    for i in range(len(attributes[0])):
        row = []
        for j in range(len(attributes[0])):
            matchable = True
            for k in range(len(attributes[0][i])):
                if attributes[0][i][k] == attributes[1][j][k]:
                    matchable = False
            row.append(matchable)
        
        fullCompatibilityMatrix.append(row)
        
    #print(fullCompatibilityMatrix)
    seasonId = idToString(generateSortedId(fullCompatibilityMatrix)[0])
    #print(generateSortedId(fullCompatibilityMatrix))
    if seasonId not in seasonLog:
        seasonLog[seasonId] = set()
            
def sortMatrix(matrix, rowOrder, columnOrder, inverse=False):
    result = [[0]* len(rowOrder) for _ in range(len(rowOrder))]
    for i in range(len(rowOrder)):
        for j in range(len(rowOrder)):
            if not inverse:
                result[i][j] = matrix[rowOrder[i]][columnOrder[j]]
            else:
                result[rowOrder[i]][columnOrder[j]] = matrix[i][j]

    return result


def generateUnsortedId(matrix, order, rowMode):
    id = []
    matrix_size = len(matrix)
    for i in range(matrix_size):
        temp = 0
        for j in range(matrix_size):
            temp<<=1
            entry = matrix[i][j] if rowMode else matrix[j][i]
            if entry:
                temp |= 1
        id.append((temp, order[i]))
    return id

def generateSortedId(compatibilityMatrix):
    rowOrder = []
    columnOrder = []
    for i in range(len(compatibilityMatrix)):
        rowOrder.append(i)
        columnOrder.append(i)
    matrix2 = compatibilityMatrix
    row = True
    sorted = [False, False]
    while True:
        order = rowOrder if row else columnOrder
        subId = generateUnsortedId(matrix2, order, row)
        sorted[0 if row else 1] = True
        maximum = -1
        for i in range(len(subId)):
            if subId[i][0] < maximum:
                sorted[0 if row else 1] = False
                break
            else:
                maximum = subId[i][0]
        if not sorted[0 if row else 1]:
            subId.sort(key=lambda x: x[0])
            for i in range(len(subId)):
                order[i] = subId[i][1]
        if row:
            id = subId
        if sorted[0] and sorted[1]:
            break
        matrix2 = sortMatrix(compatibilityMatrix, rowOrder, columnOrder)
        row = not row
    key = []
    for i in range(len(id)):
        key.append(id[i][0])
    return [key, rowOrder, columnOrder]

def idToString(id):
    s = ''
    for i in range(len(id)):
        if id[i] < 16:
            s += f'000{id[i]:x}'
        elif id[i] < 256:
            s += f'00{id[i]:x}'
        elif id[i] < 4096:
            s += f'0{id[i]:x}'
        else:
            s += f'{id[i]:x}'
    return s

def loadProbabilities(id):
    s = idToString(id[0])
    seasonLog[seasonId].add(s)
    temp = computedProbabilities.get(s)
    
    if (temp is None) or (temp == []):
        return temp
    
    probabilities = sortMatrix(temp, id[1], id[2], True)
    return probabilities

def saveProbabilities(id, probabilities):
    s = idToString(id[0])
    
    if (probabilities is None) or (probabilities == []):
        computedProbabilities[s] = probabilities
    else:
        temp = sortMatrix(probabilities, id[1], id[2])
        computedProbabilities[s] = temp
        
count = 0
matrices = []
        
def computeProbabilities(compatibilityMatrix, unmatchedRunnerUp=None):
    global count, matrices
    #matrices.add(compatibilityMatrix)
    count += 1
    matrices.append(compatibilityMatrix)
    size = len(compatibilityMatrix)
    id = None

    if unmatchedRunnerUp is None:
        # Use cached probabilities if existing
        id = generateSortedId(compatibilityMatrix)
        cachedProbabilities = loadProbabilities(id)
        if cachedProbabilities is not None:
            return cachedProbabilities

    probabilities = [[0] * size for _ in range(size)]
    options = 0

    if unmatchedRunnerUp is None:
        for i in range(size):
            options += 1
            # Temporarily draw runner-up i and compute the resulting probabilities
            conditionalProbabilities = computeProbabilities(compatibilityMatrix, i)
            if conditionalProbabilities is None:
                options -= 1
            else:
                for j in range(size):
                    for k in range(size):
                        probabilities[j][k] += conditionalProbabilities[j][k]

        # Return None if the current draw is a dead end
        if options == 0 and size > 0:
            probabilities = None

    else:
        for i in range(size):
            if compatibilityMatrix[i][unmatchedRunnerUp]:
                options += 1
                # Temporarily match unmatchedRunnerUp with winner i and compute the resulting probabilities
                subMatrix = []
                for j in range(size):
                    if j != i:
                        row = []
                        for k in range(size):
                            if k != unmatchedRunnerUp:
                                row.append(compatibilityMatrix[j][k])
                        subMatrix.append(row)
                                
                
                #subMatrix = [
                #    [compatibilityMatrix[j][k] for k in range(size) if k != unmatchedRunnerUp]
                #    for j in range(size) if j != i
                #]
                conditionalProbabilities = computeProbabilities(subMatrix)
                if conditionalProbabilities is None:
                    options -= 1
                else:
                    for j in range(size):
                        for k in range(size):
                            if j < i:
                                if k < unmatchedRunnerUp:
                                    probabilities[j][k] += conditionalProbabilities[j][k]
                                if k > unmatchedRunnerUp:
                                    probabilities[j][k] += conditionalProbabilities[j][k - 1]
                            elif j > i:
                                if k < unmatchedRunnerUp:
                                    probabilities[j][k] += conditionalProbabilities[j - 1][k]
                                if k > unmatchedRunnerUp:
                                    probabilities[j][k] += conditionalProbabilities[j - 1][k - 1]
                    probabilities[i][unmatchedRunnerUp] += 1

        # Return None if the current draw is a dead end
        if options == 0:
            probabilities = None

    if options != 0:
        for i in range(size):
            for j in range(size):
                probabilities[i][j] /= options

    if unmatchedRunnerUp is None:
        saveProbabilities(id, probabilities)
    return probabilities

def getProbabilities(drawnW = None, drawnR = None, unmatchedRunnerUp = None):
    if drawnW is None:
        return computeProbabilities(fullCompatibilityMatrix)

    compatibilityMatrix = []
    size = len(fullCompatibilityMatrix)

    for i in range(size):
        if not drawnW[i]:
            row = []
            for j in range(size):
                if not drawnR[j] or j == unmatchedRunnerUp:
                    if fullCompatibilityMatrix[i][j]:
                        row.append(True)
                    else:
                        row.append(False)
            compatibilityMatrix.append(row)

    i = unmatchedRunnerUp - 1
    while i >= 0:
        if drawnR[i]:
            unmatchedRunnerUp -= 1
        i -= 1

    return computeProbabilities(compatibilityMatrix, unmatchedRunnerUp)


winners = [["A","DE"], ["B","EN"], ["C","ES"], ["D","ES"], ["E","ES"], ["F","DE"], ["G","EN"], ["H","ES"]]
runnersUp = [["A","DEN"], ["B","NE"], ["C","IT"], ["D","IT"], ["E","IT"], ["F","FR"], ["G","DE"], ["H","PR"]]
initialize(winners, runnersUp)

start = time.perf_counter()
a = getProbabilities()
end = time.perf_counter()
print(end-start)
print(a)


