import pandas as pd

ignore_chars = ['.', ',', '!', '?', ';', ':', '-', '(', ')', '"', "'","\n"," "]

# importa o coteudo do arquivo txt que esta cifrado como UTF-8
texto_cifrado = ''
with open('cipher.txt', 'r', encoding='utf-8') as f:
    texto_cifrado = f.read()

# importa as frequencias das letras em portugues
frequencias = pd.read_csv('freqs.csv', sep=',')
try:
    frequencias = frequencias.sort_values(by='Freq', ascending=False).reset_index(drop=True)
    # dropa a coluna de frequencias
    frequencias = frequencias.drop(columns=['Freq'])
    # salva as frequencias
    frequencias.to_csv('freqs.csv', index=False)
except:
    pass


# verifica se freq_cipher existe, se existir carrega
try:
    frequencias_cifrado = pd.read_csv('freq_cipher.csv', sep=',')
except:
    # pega as frequencias das letras no texto cifrado
    frequencias_cifrado = pd.DataFrame({'Letra': list(texto_cifrado)})
    # remove os caracteres que nao sao letras
    frequencias_cifrado = frequencias_cifrado[~frequencias_cifrado.Letra.isin(ignore_chars)]

    frequencias_cifrado = frequencias_cifrado.groupby('Letra').size().reset_index(name='Freq')
    frequencias_cifrado['Freq'] = 100 * frequencias_cifrado['Freq'] / frequencias_cifrado['Freq'].sum()

    # coloca as frequencias em capital letters
    frequencias_cifrado['Letra'] = frequencias_cifrado['Letra'].str.upper()

    # ordena as frequencias e cria um novo index
    frequencias_cifrado = frequencias_cifrado.sort_values(by='Freq', ascending=False).reset_index(drop=True)

    # dropa a coluna de frequencias
    frequencias_cifrado = frequencias_cifrado.drop(columns=['Freq'])
    # salva as frequencias
    frequencias_cifrado.to_csv('freq_cipher.csv', index=False)



print(frequencias_cifrado)
print("----")
print(frequencias)

print("================\ndecifrando\n\n\n")

# cria o texto decifrado
texto_decifrado = ''
for letra in texto_cifrado:
    if letra in ignore_chars:
        texto_decifrado += letra
    else:
        print(f'letra cifrada: {letra} -> ', end='')
        index = frequencias_cifrado[frequencias_cifrado.Letra == letra.upper()].index[0]
        print(f'index: {index} -> letra decifrada: {frequencias.iloc[index]["Letra"]}')
        texto_decifrado += frequencias.iloc[index]['Letra']

# salva o texto decifrado
with open('decipher.txt', 'w') as f:
    f.write(texto_decifrado)