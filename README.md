# Como Adicionar Novos Caracteres ao Display OLED com a Biblioteca SSD1306

Este tutorial explica como adicionar novos caracteres (como 'Ç', 'Ã', etc.) à biblioteca SSD1306 para exibi-los em um display OLED. Siga os passos abaixo para implementar caracteres personalizados.

## Passo 1: Desenhar o Caractere

Acesse a ferramenta **Online LED Matrix Font Generator**:

🔗 [Online LED Matrix Font Generator](https://www.example.com)

### Configure a ferramenta:
- Defina o tamanho da matriz como **8x8** (ou o tamanho da fonte que você está usando).
- Certifique-se de que a orientação do desenho seja **horizontal** (o caractere deve ser desenhado da esquerda para a direita).

### Desenhe o caractere:
- Use o mouse para clicar nos pixels que devem estar acesos.
- O caractere será exibido na grade ao lado.
- Copie o valor hexadecimal gerado.

## Passo 2: Adicionar o Caractere ao Arquivo `ssd1306_font.h`

1. Abra o arquivo `ssd1306_font.h`.
2. Localize a matriz `static uint8_t font[] = {...}`.
3. Adicione os valores hexadecimais do novo caractere ao final da matriz:

### Exemplo:
Se você desenhou o caractere 'Ç' e obteve os seguintes valores hexadecimais:
```c
0x7e, 0x41, 0x41, 0xc1, 0x41, 0x41, 0x41, 0x00
```
Adicione-os à matriz `font[]`:
```c
static uint8_t font[] = {
    // ... (caracteres existentes)
    0x7e, 0x41, 0x41, 0xc1, 0x41, 0x41, 0x41, 0x00, // Caractere 'Ç'
};
```

## Passo 3: Obter o Valor Hexadecimal do Caractere

Acesse a ferramenta **ASCII to Hex Converter**:

🔗 [ASCII to Hex Converter](https://www.example.com)

1. Digite o caractere que deseja adicionar (por exemplo, 'Ç').
2. Anote o valor hexadecimal correspondente:
   - Para 'Ç', o valor é **0xC7**.
   - Nesse sentido, a sequência de escape será: **\xC7**

**Nota:** A sequência de escape é um conceito em programação que permite representar caracteres especiais ou não imprimíveis em strings ou literais de caractere. Em C (e muitas outras linguagens de programação), uma sequência de escape começa com uma barra invertida '\' seguida de um ou mais caracteres que definem o significado especial (nesse caso, o valor hexadecimal correspondente ao novo caractere).

## Passo 4: Tratar o Caractere no Arquivo `ssd1306_i2c.c`

1. Abra o arquivo `ssd1306_i2c.c`.
2. Localize a função `inline int ssd1306_get_font(uint8_t character)`.
3. Adicione um `else if` para mapear o novo caractere ao índice correspondente na matriz `font[]`.

### Exemplo:
Se o caractere 'Ç' foi adicionado ao final da matriz `font[]`, ele terá o índice **38** (considerando que os caracteres anteriores ocupam os índices 0 a 37 - O 'hífen' e o 'Ã' também foram adicionados no exemplo).

```c
inline int ssd1306_get_font(uint8_t character)
{
    if (character >= 'A' && character <= 'Z')
    {
        return character - 'A' + 1;
    }
    else if (character >= '0' && character <= '9')
    {
        return character - '0' + 27;
    }
    else if (character == '-')
    {
        return 37;
    }
    else if (character == '\xC7') // Ç
    {
        return 38;
    }
    else if (character == '\xC3') // Ã
    {
        return 39;
    }
    else
        return 0;
}
```

## Passo 5: Usar o Caractere no Arquivo Principal

No arquivo principal (onde você define os textos), use a sequência de escape `\x` para incluir o caractere especial.

### Exemplo:
Se você deseja exibir o texto **ATENÇÃO** com os caracteres 'Ç' e 'Ã', faça o seguinte:
```c
void SinalAtencaoPedestre()
{
    gpio_put(LED_R_PIN, 1);
    gpio_put(LED_G_PIN, 1);
    gpio_put(LED_B_PIN, 0);

    const char *text[] = {
        "SINAL DE ",
        "ATEN\xC7\xC3O - ",  // 'Ç' e 'Ã' com sequências de escape
        "APRESSE-SE"};
    display_text(text, 3);
}
```

## Passo 6: Compilar e Testar

1. Compile o código e carregue-o no microcontrolador.
2. Verifique se o novo caractere é exibido corretamente no display OLED.

---

## 🔥 **Resumo dos Passos**

| Passo | Ação |
|-------|------|
| **1** | Desenhar o caractere na ferramenta [Online LED Matrix Font Generator](https://www.example.com) e copiar os valores hexadecimais. |
| **2** | Adicionar os valores hexadecimais à matriz `font[]` no arquivo `ssd1306_font.h`. |
| **3** | Obter o valor hexadecimal do caractere usando a ferramenta [ASCII to Hex Converter](https://www.example.com). |
| **4** | Mapear o caractere na função `ssd1306_get_font` no arquivo `ssd1306_i2c.c`. |
| **5** | Usar a sequência de escape `\x` para incluir o caractere no texto. |
| **6** | Compilar e testar. |

---

## 🎯 **Exemplo Completo**

### Arquivo `ssd1306_font.h`:
```c
static uint8_t font[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Nothing
    0x78, 0x14, 0x12, 0x11, 0x12, 0x14, 0x78, 0x00, // A
    0x7f, 0x49, 0x49, 0x49, 0x49, 0x49, 0x7f, 0x00, // B
    0x7e, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x00, // C
    0x7f, 0x41, 0x41, 0x41, 0x41, 0x41, 0x7e, 0x00, // D
    0x7f, 0x49, 0x49, 0x49, 0x49, 0x49, 0x49, 0x00, // E
    0x7f, 0x09, 0x09, 0x09, 0x09, 0x01, 0x01, 0x00, // F
    0x7f, 0x41, 0x41, 0x41, 0x51, 0x51, 0x73, 0x00, // G
    0x7f, 0x08, 0x08, 0x08, 0x08, 0x08, 0x7f, 0x00, // H
    0x00, 0x00, 0x00, 0x7f, 0x00, 0x00, 0x00, 0x00, // I
    0x21, 0x41, 0x41, 0x3f, 0x01, 0x01, 0x01, 0x00, // J
    0x00, 0x7f, 0x08, 0x08, 0x14, 0x22, 0x41, 0x00, // K
    0x7f, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x00, // L
    0x7f, 0x02, 0x04, 0x08, 0x04, 0x02, 0x7f, 0x00, // M
    0x7f, 0x02, 0x04, 0x08, 0x10, 0x20, 0x7f, 0x00, // N
    0x3e, 0x41, 0x41, 0x41, 0x41, 0x41, 0x3e, 0x00, // O
    0x7f, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0e, 0x00, // P
    0x3e, 0x41, 0x41, 0x49, 0x51, 0x61, 0x7e, 0x00, // Q
    0x7f, 0x11, 0x11, 0x11, 0x31, 0x51, 0x0e, 0x00, // R
    0x46, 0x49, 0x49, 0x49, 0x49, 0x30, 0x00, 0x00, // S
    0x01, 0x01, 0x01, 0x7f, 0x01, 0x01, 0x01, 0x00, // T
    0x3f, 0x40, 0x40, 0x40, 0x40, 0x40, 0x3f, 0x00, // U
    0x0f, 0x10, 0x20, 0x40, 0x20, 0x10, 0x0f, 0x00, // V
    0x7f, 0x20, 0x10, 0x08, 0x10, 0x20, 0x7f, 0x00, // W
    0x00, 0x41, 0x22, 0x14, 0x14, 0x22, 0x41, 0x00, // X
    0x01, 0x02, 0x04, 0x78, 0x04, 0x02, 0x01, 0x00, // Y
    0x41, 0x61, 0x59, 0x45, 0x43, 0x41, 0x00, 0x00, // Z
    0x3e, 0x41, 0x41, 0x49, 0x41, 0x41, 0x3e, 0x00, // 0
    0x00, 0x00, 0x42, 0x7f, 0x40, 0x00, 0x00, 0x00, // 1
    0x30, 0x49, 0x49, 0x49, 0x49, 0x46, 0x00, 0x00, // 2
    0x49, 0x49, 0x49, 0x49, 0x49, 0x49, 0x36, 0x00, // 3
    0x3f, 0x20, 0x20, 0x78, 0x20, 0x20, 0x00, 0x00, // 4
    0x4f, 0x49, 0x49, 0x49, 0x49, 0x30, 0x00, 0x00, // 5
    0x3f, 0x48, 0x48, 0x48, 0x48, 0x48, 0x30, 0x00, // 6
    0x01, 0x01, 0x01, 0x61, 0x31, 0x0d, 0x03, 0x00, // 7
    0x36, 0x49, 0x49, 0x49, 0x49, 0x49, 0x36, 0x00, // 8
    0x06, 0x09, 0x09, 0x09, 0x09, 0x09, 0x7f, 0x00, // 9
    0x00, 0x08, 0x08, 0x08, 0x08, 0x08, 0x00, 0x00, // hífen (-)
    0x7e, 0x41, 0x41, 0xc1, 0x41, 0x41, 0x41, 0x00, // Ç
    0xf0, 0x28, 0x25, 0x23, 0x25, 0x28, 0xf0, 0x00, // Ã
};
```

### Arquivo `ssd1306_i2c.c`:
```c
inline int ssd1306_get_font(uint8_t character)
{
    if (character >= 'A' && character <= 'Z')
    {
        return character - 'A' + 1;
    }
    else if (character >= '0' && character <= '9')
    {
        return character - '0' + 27;
    }
    else if (character == '-')
    {
        return 37;
    }
    else if (character == '\xC7')
    {
        return 38;
    }
    else if (character == '\xC3')
    {
        return 39;
    }
    else
        return 0;
}
```

### Arquivo Principal:
```c
void SinalAtencaoPedestre()
{
    gpio_put(LED_R_PIN, 1);
    gpio_put(LED_G_PIN, 1);
    gpio_put(LED_B_PIN, 0);

    const char *text[] = {
        "SINAL DE ",
        "ATEN\xC7\xC3O - ",
        "APRESSE-SE"};
    display_text(text, 3);
}
```

🚀 Com este tutorial, você pode adicionar qualquer caractere especial ao seu display OLED!

📌 **Nota:** Certifique-se de que o arquivo esteja salvo com codificação **UTF-8** para evitar problemas com caracteres especiais.
