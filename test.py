from random import randint, choice

valores = set()

ops = ['IMP', 'INC', 'INC', 'INC', 'REM', 'SUC', 'PRE']

SIZE = 1000
MAX_VAL = 4294967295

entrada = ''
saida = ''

for i in range(SIZE): 
    op = choice(ops)
    
    if op == 'INC':
        x = randint(0, MAX_VAL)
        entrada += f"INC {x}\n"
        valores.add(x)
    elif op == 'REM':
        remover_valor_existente = True
        if remover_valor_existente and valores:
            x = choice(list(valores))
            entrada += f"REM {x}\n"
            valores.remove(x)
        else:
            x = randint(0, MAX_VAL)
            
            if x in valores:
                valores.remove(x)
            
            entrada += f"REM {x}\n"
            
    elif op == 'SUC':
        x = randint(0, MAX_VAL)
        
        entrada += f"SUC {x}\n"
        saida += f"SUC {x}\n"
        
        if valores:
            suc = min((v for v in valores if v > x), default=None)
            if suc is not None:
                saida += f"{suc}\n"
            else:
                saida += f"+INF\n"
        else:
            saida += f"+INF\n"
        
    elif op == 'PRE':
        x = randint(0, MAX_VAL)
        
        entrada += f"PRE {x}\n"
        saida += f"PRE {x}\n"
        
        if valores:
            pre = max((v for v in valores if v < x), default=None)
            if pre is not None:
                saida += f"{pre}\n"
            else:
                saida += f"-INF\n"
        else:
            saida += f"-INF\n"
    elif op == 'IMP':
        entrada += "IMP\n"
        
        saida += "IMP\n"
        
        if valores:
            saida += f'Min: {min(valores)}'
        
        C = {}
        
        for v in list(sorted(valores))[1:]:
            cluster = v >> 16
            
            if not cluster in C:
                C[cluster] = []
            
            C[cluster].append(v & 0xFFFF)
            
        for c in sorted(C.keys()):
            saida += f", C[{c}]: {C[c][0]}"
            for v in C[c][1:]:
                saida += f", {v}"
                
        saida += "\n"

import subprocess

arquivo = 'test.txt'

with open(arquivo, 'w') as f:
    f.write(entrada)

res = subprocess.run(['./programa.exe', arquivo], capture_output=True, text=True)

print('C++ Output:')
print(res.stdout)

print('-' * 20)
print('PY:')
print(saida)

print(res.stdout == saida)