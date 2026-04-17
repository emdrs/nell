# Comentários

// Comentário de linha unica
/*
    Comentário
    de
    múltiplas
    linhas
*/

# Variáveis

nome: tipo // Sem inicialização
nome: tipo = valor
nome := valor

num := 10 // Entende que é int
num: i64 = 10 // Especifica que é int 64 bits

# Constantes

num :: 10

# Ponteiros

nome: *tipo = valor

num: *int = alloc(int, 3)

# Alguns tipos

- int
- float
Talvez variações: i32, f16...
- str
- Vector2
- Vector3

## Arrays
nome: tipo[tamanho]
nome: int[3] = [10, 20, 30] // Talvez um alias para 
nome := [10, 20, 30] // Isso define uma lista

## Lista dinâmica

nome: [tipo]
nome := [valores]
nome := [10, 20, 30]

## Dicionário

nome: [tipo chave: tipo valor] = [chave: valor ...]
nome: [str: int] = ["a": 10, "b": 20]
nome := [chave: valor ...]
nome := ["a": 10, "b": 20]

# Condicionais

if condicao { } else if condicao { } else { }
if a == 10 { } else if a == 5 { } else { }

// Variáveis usadas somente no escopo
if valor := x { } else if a == 5 { } else { }

// Switch case

if variavel {
    case caso { }
    case _ { } // Default
}

if num {
    case 10 { }
    case _ { }
}

# Repetição

0 e 1 inclusivos
for nome : inicio..fim { }
for i : 0..1 { }
for nome : inicio..fim..step { }
for i : 0..1..1 { }

while condicao { }

Variáveis usadas somente no escopo
while playing := true { }

break    // Sai do laço
continue // Pula a iteração

# Funções

return // Retorna valor ou sai

nome :: (parametros) -> retorno { }
nome :: (parametros) { } // Sem retorno
soma :: (a: int, b: int) -> int { return a + b }

## Execução

soma(10, 5)

// Parametros nomeados

soma(b: 10, a: 20)

# Structs

struct nome {
    variavel: tipo

    // Construtor, deve inicializar todos as variáveis, senão erro
    init() { }
}

// Caso não tenha um construtor, uma função default que pede todos os dados é criada
// com o mesmo nome da struct

## Métodos

Pessoa :: struct {
    nome: str
    idade: int = 10 // Valor inicializado antes do construtor

    DONO :: Pessoa("Borges", 25)

    // Em c injeta um parametro no início que é um ponteiro de Pessoa
    falar :: (mensagem: str) {
        print(self.nome, ": ", mensagem)
    }
}

// Supondo
teste :: (pessoa: Pessoa) { }
teste(Pessoa.DONO)
// Pode-se executar
teste(.DONO)

## Instancia

struct(parametros)

pessoa1 := Pessoa("Emanuel")

// Ponteiro

pessoa2 := new Pessoa("Manel")

pessoa1.falar("Oi")
pessoa2->falar("Opa")

# Conversões (casting)

pessoa3 := (*Pessoa) alloc(Pessoa, 10)

# Enumeradores

Nome :: enum {
    valores
}

HTTPMethods :: enum {
    GET;
    POST;
    PUT;
    DELETE;
    OPTIONS;
}

## Uso

metodo: HTTPMethods = .GET

# Imports

#import "Biblioteca"

// Caso tenha conflito:
Biblioteca = #import "Biblioteca"

Biblioteca.funcao()
