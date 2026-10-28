# SoulSilver no Xbox One S apenas com celular

Este projeto tem duas rotas experimentais, **nenhuma ainda validada como jogável
em um Xbox One S real**. Ambas usam o Xbox em Developer Mode e o navegador do
celular conectado à mesma rede local do console.

## Rota A — Beta de interpretador, sem enviar sua ROM ao GitHub (tente primeiro)

O workflow público `Xbox One S single-screen interpreter beta APPX (NO ROM)`
compila a interface `merged` do ssrecomp com o interpretador ARM9 do melonDS.
Isto NÃO é a recompilação estática completa e pode ter desempenho inferior.

1. Abra [o workflow de beta](https://github.com/a26773610-code/ssrecomp/actions/workflows/xbox-uwp-interpreter-beta.yml).
2. Abra uma execução que esteja **Success**; em **Artifacts**, baixe
   `xbox-one-s-ssrecomp-single-screen-interpreter-beta-no-rom`.
   Se não existir execução aprovada, não há beta pronta.
3. No celular, extraia o ZIP. O pacote contém um `.appx`, seu certificado
   `.cer` e, quando disponível, a dependência `Microsoft.VCLibs`.
4. Conecte o celular à rede do Xbox; no Dev Home do Xbox, habilite
   **Xbox Device Portal**. Abra o endereço e a porta informados pelo console
   no navegador do celular. Em **My games & apps / Add**, instale o APPX,
   fornecendo certificado/dependências caso exigidos.
5. Renomeie uma cópia de sua ROM pessoal compatível para `rom.nds`.
   No Device Portal, em **File explorer**, selecione a aplicação
   `SSRecomp.XboxInterpreterBeta`, abra `LocalAppData/LocalState` e carregue
   `rom.nds` diretamente do armazenamento do celular.
6. Inicie o aplicativo. Sem ROM, ele deverá mostrar uma tela de instruções;
   após o upload, tentará entrar no jogo. Salvas: `soulsilver.sav`.
   Log: `ssrecomp-game.log` na mesma pasta. Instalação, boot,
   velocidade, entrada de controles, áudio e salvamento exigem teste real.

**Não publique sua ROM:** nesta rota ela vai apenas do celular para o
Xbox Device Portal. Não há necessidade de enviá-la por chat ou ao GitHub.

## Rota B — Recompilação estática privada no GitHub Actions

Esta rota só é necessária se a beta com interpretador não funcionar ou se
quisermos testar a versão ARM9 recompilada. O `ssrecomp` incorpora bytes
específicos de código da ROM ao executável. A ROM e o APPX são dados
privados, e NÃO devem ser carregados em um repositório público.

1. Pelo navegador do celular, abra https://github.com/new/import.
2. Na origem, informe
   `https://github.com/a26773610-code/ssrecomp.git`.
3. Escolha um nome distinto, por exemplo `ssrecomp-xbox-private`, e
   obrigatoriamente visibilidade **Private**. Faça **Begin import**.
   **Não use Fork público**.
4. No novo repositório privado, escolha o branch `xbox-uwp-port` e defina-o
   como branch padrão em **Settings > Default branch**, para que
   `workflow_dispatch` apareça na aba Actions. Aguarde a importação
   finalizar antes de alterar o branch padrão.
5. Em **Releases > Draft a new release**, crie a tag exata `soulrom-input`
   apontando para esse branch; anexe **um** arquivo `.nds` compatível do
   seu celular como asset (arquivo binário), e publique a release no
   **repositório PRIVADO**. Não faça upload via `Add file` / commit.
6. Em **Actions**, selecione
   `Build private SoulSilver Xbox UWP APPX (cellphone workflow)`,
   escolha **Run workflow** no branch `xbox-uwp-port`.
7. O runner Windows baixará o asset `.nds` dessa mesma release privada,
   verificará os 121 hashes, gerará os cerca de 368 MB de código privado,
   compilará com Visual Studio C++ UWP x64 e tentará assinar o APPX.
   Se a compilação falhar, leia o log privado; essa etapa completa
   AINDA NÃO foi aprovada para o jogo real.
8. Se a compilação terminar em **Success**, baixe o artifact
   `PRIVATE-SoulSilver-Xbox-APPX-not-public` (retido por um dia).
   Extraia APPX e CER, instale via Device Portal, depois mande sua cópia
   `rom.nds` para `LocalState` da aplicação `SSRecomp.XboxSoulSilver`.

IMPORTANTE: uma release em repositório privado fica restrita aos usuários
autorizados, mas a ROM passa a ser armazenada nos servidores do GitHub.
Use apenas ROM própria e legalmente obtida, aceite conscientemente esse
armazenamento e exclua a release contendo a ROM quando não precisar mais.
Os artefatos que contêm código derivado da ROM também são privados.

**Guardas da rota B:** o workflow verifica pelo GitHub API que o repositório é
privado e só executa com `github.event.repository.private == true`.
`GITHUB_TOKEN` é limitado a `contents: read`. Não contém URL aberta,
senhas ou ROM pública. O upload do APPX é um artefato privado temporário.
