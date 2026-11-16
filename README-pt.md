# MASMORRA

[English](README.md) | Português

[![Linguagem: C](https://img.shields.io/badge/linguagem-C-blue.svg)](#início-rápido)
[![Zero Dependências](https://img.shields.io/badge/depend%C3%AAncias-zero%20(stdlib)-brightgreen.svg)](#início-rápido)
[![Plataforma](https://img.shields.io/badge/plataforma-Linux%20%7C%20macOS%20%7C%20Windows-lightgrey.svg)](#início-rápido)
[![Terminal ANSI](https://img.shields.io/badge/ANSI-256%20%7C%2016%20%7C%20Mono-yellow.svg)](#opções-de-cor)

Um clássico *Dungeon Crawler* de terminal baseado em turnos, encapsulado em um único arquivo C monolítico — sem dependências externas, compilação multiplataforma, detecção de cores ANSI em tempo de execução, 5 andares de masmorra, melhorias em lojas, emboscadas de mímicos e três finais.

![MASMORRA](assets/masmorra_title.gif)

## Início Rápido

### Baixar e Jogar (Recomendado)

Baixe o binário pré-compilado na [página de Releases](https://github.com/paixonite/MASMORRA/releases):

- **Windows:** Baixe `masmorra_windows.exe` e dê um duplo clique (ou execute pelo Prompt de Comando / PowerShell).
- **Linux:** Baixe `masmorra_linux`, dê permissão de execução e execute:

```bash
chmod +x masmorra_linux
./masmorra_linux
```

### Compilar a partir do Código-Fonte

A compilação é instantânea com qualquer compilador C padrão (`gcc`, `clang` ou MinGW):

#### Linux / macOS

```bash
git clone https://github.com/paixonite/MASMORRA.git
cd MASMORRA
gcc MASMORRA.c -o masmorra -Wall
./masmorra
```

#### Windows (MinGW)

```cmd
gcc MASMORRA.c -o masmorra.exe -Wall
masmorra.exe
```

## Opções de Cor

As cores do terminal são detectadas automaticamente em tempo de execução com fallback para monocromático, e podem ser substituídas via flags de linha de comando:

```bash
./masmorra --color      # Força a saída de cores ANSI
./masmorra --no-color   # Desativa as cores ANSI (modo monocromático)
```

## Autor

- **Vítor Paixão** - [https://github.com/paixonite](https://github.com/paixonite)
