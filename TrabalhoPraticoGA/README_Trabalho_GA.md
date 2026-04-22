# Computação Gráfica Avançada - 2026/1 - TRABALHO GA

Autores: Artur Pires, Marina Oliveira e Rodrigo de Moraes Lehnen

--------------------------------------------------------------

# Introdução

Neste trabalho foi construída uma cena 3D no Blender com o objetivo de comparar dois métodos de renderização: tempo real (Eevee) e path tracing (Cycles).

A cena foi planejada para evidenciar fenômenos importantes de iluminação, incluindo:

[x] Múltiplas fontes de luz
[x] Sombras projetadas
[x] Reflexões especulares
[x] Iluminação indireta (global illumination)

O foco principal foi demonstrar como o Cycles, baseado em path tracing, consegue simular o comportamento físico da luz de forma mais realista em comparação ao pipeline rasterizado do Eevee.

# Metodologia

A cena foi modelada no próprio Blender utilizando Modelagem para alguns elementos, e a implementação de assets prontos, para outros com objetivo de tornar a cena mais realista, e materiais com diferentes propriedades (difuso, metálico e vidro).

Ferramentas e configuração:
* Software: Blender
* Render em tempo real: Eevee
* Render físico: Cycles (Path Tracing)

Montagem da cena:
* Inserção de múltiplas fontes de luz (Lâmpadas de LED, Global e Luzes de Ponto)
* Posicionamento estratégico para gerar sombras e iluminação indireta
* Aplicação de materiais com diferentes propriedades:
  - Superfícies reflexivas (metal)
  - Superfícies refrativas (vidro)
  - Superfícies difusas

Path Tracing (Cycles):
O Cycles utiliza o método de path tracing, que simula o caminho da luz através de múltiplos bounces (reflexões/refratações). Para isso:

  1. Foram configurados múltiplos samples
  2. Ativado cálculo de Global Illumination
  3. Ajustados bounces de luz (difuso, glossy e transmissão)

Esse processo permite capturar efeitos como:

   1. color bleeding
   2. sombras suaves fisicamente corretas
   3. reflexões múltiplas

# Considerações Finais

O path tracing do Cycles captura fenômenos que o tempo real não consegue representar completamente, como:

* Múltiplas interações da luz (bounces)
* Iluminação indireta fisicamente correta
* Reflexões globais precisas
* Refração realista em materiais transparentes

Limitações:

Cycles (Path Tracing):
* Alto custo computacional
* Tempo de render elevado
* Presença de ruído com poucos samples

Eevee (Tempo real):
* Menor realismo físico
* Limitações em iluminação indireta
* Reflexões dependentes da tela (screen space)

🧠 Conclusão geral

Enquanto o Eevee prioriza desempenho e interatividade, o Cycles prioriza precisão física. A escolha entre os dois depende do objetivo: velocidade vs realismo.

# Lista de Assets

Privada - https://sketchfab.com/3d-models/toilet-f4d2db1700f64d04be3e090716911475
Shampoo - https://sketchfab.com/3d-models/shampoo-bottle-38508e5df44840ebae254e40e1ebd73f
Sabonete - https://sketchfab.com/3d-models/soap-78818d260eec47058cba2ee469af8904
Lixeira - https://sketchfab.com/3d-models/lixeira-33f3114094bd40a9b04788e4779e338b
Papel Higiênico e Suporte - https://sketchfab.com/3d-models/toilet-paper-holder-d3c954cf50c746a3a8fa5dda648f855f
Pia - https://sketchfab.com/3d-models/bathroom-vanity-with-marble-topfree-0c183eb640234ab38bd64980458d65e3
Toalha de Rosto e Suporte - https://sketchfab.com/3d-models/porta-toalha-de-rosto-a86074116df64f85b051d661a203d0a6
Chuveiro - https://www.blenderkit.com/asset-gallery-detail/37d04c22-6ba5-4597-8a46-fa748cb56669/?query=category_subtree:model+shower+order:_score
Ralo - https://www.blenderkit.com/asset-gallery-detail/0728a864-b479-4364-ae0c-86499fd264af/?query=category_subtree:model+shower+order:_score+availability:free
Pasta de Dente - https://www.blenderkit.com/asset-gallery-detail/83fcc23b-e4b5-459a-b610-d9317aaa7638/?query=category_subtree:model+shower+order:_score+availability:free
Tapete - https://sketchfab.com/3d-models/petate-carpet-straw-carpet-straw-rug-1a5021f8623c4ddeb5eabf7eccb77bc2