def mediana(A, B, n):
    if n == 1:
        return min(A[0], B[0])
    
    q = n // 2
    
    if n % 2 == 0:
        if A[q-1] < B[q-1]:
            return mediana(A[q:], B[:q], n - q)
        else:
            return mediana(A[:q], B[q:], n - q)
    else:
        if A[q] < B[q]:
            return mediana(A[q:], B[:q+1], n - q)
        else:
            return mediana(A[:q+1], B[q:], n - q) 

from random import choices

A = sorted(choices(range(1, 100), k=50))
B = sorted(choices(range(1, 100), k=50))
    
n = len(A)

print("La mediana de los dos arreglos es:", mediana(A, B, n)) 
print('Expected median:', sorted(A + B)[n - 1])