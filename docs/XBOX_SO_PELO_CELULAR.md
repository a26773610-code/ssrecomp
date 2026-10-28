# Xbox One S: instalar e compilar só pelo celular (sem PC)

Este port é **experimental** e voltado para um Xbox One S com **Developer Mode**.
O telefone e o Xbox precisam estar na mesma rede local para usar o Xbox Device Portal.
Tudo abaixo usa GitHub Actions como computador Windows na nuvem.

## Caminho A — primeiro teste sem compilar a ROM (mais simples)

O workflow público **Xbox One S single-screen interpreter beta APPX (NO ROM)**
(`.github/workflows/xbox-uwp-interpreter-beta.yml`) compila o **ssrecomp real**,
sua interface `merged` para uma tela, SDL2 WinRT e o interpretador ARM9 melonDS,
mas **não inclui a ROM**. Isso não é recompilação estática e pode ser lento;
nem a inicialização no Xbox nem o funcionamento do jogo estão garantidos.

1. Pelo navegador do celular, abra
   https://github.com/a26773610-code/ssrecomp/actions/workflows/xbox-uwp-interpreter-beta.yml
   e procure uma execução com resultado verde **Success**.
2. Dentro da execução, na seção **Artifacts**, baixe
   `xbox-one-s-ssrecomp-single-screen-interpreter-beta-no-rom`.
3. Extraia o ZIP no Android. Ele deve conter um `.appx` do intérprete,
   o `.cer` de assinatura e, quando necessário, `Microsoft.VCLibs.x64.14.00.appx`.
   Não use o pacote `xbox-uwp-diagnostic-v04-not-game`: ele só mostra cores.
4. Ligue o Xbox em Developer Mode, abra Xbox Dev Home e ative o Device Portal.
   No celular conectado ao mesmo Wi-Fi, abra a **URL de Device Portal
   exibida na própria tela do Xbox**. Se o Chrome alertar sobre o certificado
   local, confira o IP e a porta antes de prosseguir.
5. Abra Apps/App Manager, escolha o APPX e o certificado `.cer`, adicione
   a dependência VCLibs se o portal solicitar e instale.
6. Inicie **SSRecomp Single Screen Beta** no Xbox. Com a ROM ausente, ele deve
   exibir uma tela azul-escura de instruções.
7. No Device Portal, abra **File Explorer** da aplicação instalada, encontre
   seu armazenamento **LocalState**, envie **uma cópia da sua própria ROM** e
   renomeie para `rom.nds`. O aplicativo verifica periodicamente a presença
   do arquivo e tenta iniciar automaticamente. Os saves ficam em
   `soulsilver.sav` no mesmo diretório.
8. Se voltar ao Dashboard, abra o arquivo
   `ssrecomp-game.log` em LocalState pelo Device Portal para diagnosticar.

**Importante:** este caminho só existe para chegar ao primeiro teste de execução
sem exigir um computador nem compilar código derivado da ROM em um servidor.

## Caminho B — a recompilação estática real, inteiramente no GitHub Actions

O arquivo `.github/workflows/xbox-private-soulsilver-game.yml` compila
**1,7 milhão de instruções ARM9 geradas da sua ROM** e depois tenta empacotar
um aplicativo UWP x64. Este processo pode consumir muita memória/tempo, e
a compilação real ainda não foi validada. **Não execute em repositório público.**

### Criar a cópia PRIVADA do port pelo navegador do celular

1. Entre em https://github.com/new/import e importe:
   `https://github.com/a26773610-code/ssrecomp.git`
2. Nome sugerido: `ssrecomp-xbox-pessoal`. Marque **Private** — **nunca Public**.
   O Importer copia o histórico e as branches do repositório.
3. No repositório privado, Settings > General > **Default branch**:
   troque de `main` para `xbox-uwp-port`. Isso é necessário para mostrar
   o botão **Run workflow** (o workflow precisa existir na branch padrão).
4. Em **Releases > Draft a new release**, crie uma tag `rom-input`
   apontando para `xbox-uwp-port`. No campo de arquivos binários,
   **selecione sua ROM .nds diretamente pelo seletor de arquivos do celular**.
   Não use **Add file > Upload files** do código-fonte. Releases aceitam
   arquivos binários grandes. Publique a Release **somente na cópia privada**.
   Se a interface móvel não exibir o campo, habilite a visualização
   **Site para computador** no navegador Android.
5. Abra Actions > **Xbox SoulSilver PRIVATE game APPX** > Run workflow,
   selecione a branch `xbox-uwp-port`, informe `rom-input` e
   `parallel_jobs: 2`, e execute.
6. O job se recusa a iniciar num repositório público, baixa a ROM
   autenticando com o `GITHUB_TOKEN` do próprio repositório privado,
   executa `build_game.ps1` e, se tudo funcionar, disponibiliza
   **PRIVATE-SoulSilver-Xbox-One-S-APPX-not-hardware-tested** nos Artifacts.
   Extraia o ZIP no Android e instale no Xbox pelo Device Portal, como acima.
7. Envie a mesma ROM compatível como `LocalState/rom.nds`. A ROM
   **não é genérica**: a recompilação estática do executável foi feita para
   aquela revisão do SoulSilver US em particular.

### Segurança e limites

- **NÃO** anexe a ROM a commits, issues, PRs ou releases no repositório público.
- **NÃO** publique os APPX da compilação estática: eles contêm código derivado
  do seu jogo. Compartilhe apenas o compilador, o código-fonte aberto e os testes sintéticos.
- Não envie sua senha do GitHub ou tokens a ninguém. O workflow privado usa
  o próprio `GITHUB_TOKEN`, sem PAT.
- Somente pessoas com acesso à cópia privada podem ler suas Releases e Artifacts;
  mantenha o acesso restrito. Depois de gerar o pacote, é possível excluir a
  Release `rom-input` para evitar armazenar a ROM desnecessariamente.
- Runners GitHub Actions de repositórios privados têm limites de RAM, disco
  e minutos. O compilador enorme pode falhar até ser otimizado.
- **Success no GitHub** significa que compilou/empacotou, não que o Xbox
  já testou ou que o jogo roda sem travamentos.
- A biblioteca melonDS é sob GPL; preserve as condições de distribuição
  de software e de material derivado do jogo. Use apenas sua própria ROM.
