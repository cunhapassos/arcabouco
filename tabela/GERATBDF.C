/***************************************************************************
*  $MCI M�dulo de implementa��o: GTD  Gerar tabela de defini��o
*
*  Arquivo gerado:              GERATBDF.c
*  Letras identificadoras:      GTD
*
*  Nome da base de software:    Gerar e acessar tabelas de strings
*  Arquivo da base de software: C:\AUTOTEST\PROJETOS\TABELA.BSW
*
*  Projeto: INF 1301/1628 - Exemplos
*  Gestor:  LES/DI/PUC-Rio
*  Autores: avs Arndt von Staa
*
*  $HA Hist�rico de evolu��o:
*     Vers�o  Autor    Data     Observa��es
*     1        avs  20/11/2004  Inicio desenvolvimento
*
*  $ED Descri��o do m�dulo
*     Gera a tabela de defini��o para acesso � tabela de strings residentes.
*     Uma tabela de defini��o � formada por uma lista de declara��es de
*     constantes, cada uma correspondendo � chave de acesso ao respectivo
*     string na tabela de strings. Atrav�s dessa chave as fun��es
*     TBS_ObterTamanhoString e TBS_ObterString definidas no m�dulo
*     TABSTR acessam os strings na tabela.
*
*     Para cada tabela de especifica��o de strings � gerada uma tabela
*     de defini��o. Desta forma cada m�dulo especificar� os seus strings
*     espec�ficos e dispor� da tabela de defini��o correspondente.
*     Desta forma cada m�dulo depende somente dos strings que efetivamente
*     usa.
*
*     Esta ferramenta � parte do conjunto de ferramentas que visa agilizar
*     a internacionaliza��o de programas. Os outros dois componentes
*     desse conjunto s�o:
*
*     GERATAB  - Programa gerador de tabelas de defini��o: gera as
*                constantes simb�licas a serem utilizados nos m�dulos
*                de implementa��o
*
*     TABSTR   - m�dulo de acesso � tabela de strings.
*
*  $EIU Interface com o usu�rio pessoa
*     Est�o definidos os par�metros de linha de comando:
*
*      /E<nome arquivo especifica��o>   obrigat�rio
*         este arquivo cont�m a lista de especifica��es de strings.
*
*      /D<nome arquivo defini��o>     opcional
*         este ser� o arquivo gerado. Conter� a tabela de defini��o
*         das chaves de acesso aos strings especificados.
*         O nome default do arquivo tabela �   .tabdef
*
*      /h /H /? geram o help do comando   GeraTab
*
*     Os arquivos de especifica��o de strings cont�m zero ou mais linhas.
*     Cada linha pode ser:
*
*       em branco             - � ignorada
*       // na primeira coluna - � coment�rio ignorado
*       offset de identifica��o do string
*       especifica��o de string
*
*     Uma linha de especifica��o de string tem o formato:
*
*       NomeString    idString   String
*
*          NomeString - � um nome satisfazendo a sintaxe C para uma constante
*                       simb�lica. Este nome ser� o identificador simb�lico
*                       do string.
*
*          idString   - � o valor da constante simb�lica. Os strings e
*                       os idString devem formar uma rela��o um para um.
*                       Para impedir a ocorr�ncia de idString iguais
*                       em diferentes Arquivos String, sugere-se a ado��o
*                       do seguinte padr�o de programa��o:
*
*                       - para cada m�dulo existir� no m�ximo um Arquivo
*                         String. Caso o m�dulo n�o defina string, n�o
*                         ser� necess�rio criar um Arquivo String
*                       - para cada m�dulo � definido um n�mero identificador
*                         �nico. A diferen�a de n�meros identificadores
*                         de m�dulos consecutivos deve ser sempre maior do
*                         que o n�mero de strings definidos no m�dulo
*                         de menor identificador. Sugere-se dar incrementos
*                         de pelo menos 50.
*                       - o offset de n�mero identificador � definido no
*                         arquivo de especifica��o de strings, ver
*                         declarador <Offset>
*                       - os idStrings s�o gerados consecutivamente a partir
*                         do n�mero identificador do m�dulo
*
*           String    - � um string no formato C. Valem todos os escapes
*                       padr�o de C.
*
*  $EIM Interface com outros m�dulos
*     O arquivo tabela de defini��o gerado possui uma lista de declara��es de
*     constantes inteiras.
*
***************************************************************************/

#include <stdio.h>
#include <string.h>
#include <malloc.h>

#define DIM_ID_PARM              2
#define DIM_BUFFER             512
#define DIM_NOME_ARQUIVO       512
#define DIM_NOME_STRING         34
#define DIM_STRING             256
#define DIM_STRINGAO          1023

#define LIMPA_FIM                1
#define LIMPA_INIC               2
#define CHARS_FIM                " \r\n\t"
#define ID_COMENTARIO            "//"

#define SEPARADOR_EXTENSAO       '.'
#define SEPARADOR_DIRETORIO      '\\'

const char ExtArquivoEspec[ ]  = ".espstr" ;
const char ExtArquivoTabela[ ] = ".inc" ;

const char CMD_OFFSET[ ]       = "<Offset>" ;

const char idParmArquivoEspec[ ] = "/E" ;
const char idParmTabelaDef[ ]    = "/T" ;
const char idParmAuxilio1[ ]     = "/h" ;
const char idParmAuxilio2[ ]     = "/H" ;
const char idParmAuxilio3[ ]     = "/?" ;

/***********************************************************************
*
*  $TC Tipo de dados: GT C�digos de retorno de fun��es
*
*
***********************************************************************/

   typedef enum {

         CondRetOK ,
               /* Fun��o executou correto */

         CondRetFimArq ,
               /* Fim de arquivo de leitura */

         CondRetOverflow ,
               /* Linha lida � longa demais para o buffer */

         CondRetErro
               /* Erro de leitura de arquivo */

   } tpCondRet ;

/***********************************************************************
*
*  $TC Tipo de dados: GT C�digos de t�rmino de processamento
*
*
***********************************************************************/

   typedef enum {

         CodigoOK ,
               /* Executou corretamente o programa */

         CodigoAuxilio ,
               /* Foi solicitado aux�lio */

         CodigoArquivoNaoAbre ,
               /* Arquivo n�o abre */

         CodigoNaoArquivoLista ,
               /* Arquivo lista de arquivos n�o foi definido */

         CodigoParametroDuplo ,
               /* Par�metro duplicado */

         CodigoErroProcessamento ,
               /* Erro de processamento */

         CodigoErroParametro
               /* Par�metro errado */

   } tpCodigoErro ; ;

/*****  Dados encapsulados no m�dulo  *****/

      static char NomeArquivoEspec[ DIM_BUFFER ] ;
            /* Nome do arquivo de especifica��o lido */

      static char NomeArquivoTabela[ DIM_BUFFER ] ;
            /* Nome do arquivo tabela de defini��o gerado */

      static char NomeArquivoCodigo[ DIM_BUFFER ] ;
            /* Nome de c�digo do arquivo tabela de defini��o */

      static int ContaEspecString ;
            /* Conta especifica��es de string a gerar */

      static int ContaErros    = 0 ;
            /* Conta erros ao processar GERATBDF */

/***** Prot�tipos das fun��es encapuladas no m�dulo *****/

   static void GerarTabelaDefinicao( char * NomeArquivoEspec ,
                                     char * NomeArquivoTabela ) ;

   static tpCondRet LerTrimmed( FILE * pArqLe    ,
                                char * pBuffer   ,
                                int    dimBuffer ,
                                int    Modo       ) ;

   static int ObterInxExtensao( char * NomeArquivo ) ;

/*****  C�digo das fun��es exportadas pelo m�dulo  *****/


/***********************************************************************
*
*  $FC Fun��o: GTD Processar linha de comando de gera tabela de defini��o
*
***********************************************************************/

   int main( int numParametros , char ** vtParametros  )
   {

      tpCodigoErro CodigoErro = CodigoOK ;

      char ParametroCorr[ DIM_BUFFER ] ;

      int i ;
      int numParmEspec  = 0 ;
      int numParmTabela = 0 ;

      int tamNome = 0 ;

      /* Identificar o programa gerador de tabela de defini��o */

         printf( "\nGeraTbdf - Gerar a tabela de defini��o" ) ;
         printf( "\n           LES/DI/PUC-Rio, versao 1, 05/12/2004\n" ) ;

      /* Processar todos os par�metros de GERATBDF */

         /* Processar par�metros do comando GERATBDF */

            ContaEspecString    = 0 ;
            ContaErros          = 0 ;
            numParmEspec        = 0 ;
            numParmTabela       = 0 ;

            NomeArquivoEspec[  0 ] = 0 ;
            NomeArquivoTabela[ 0 ] = 0 ;
            ParametroCorr[     0 ] = 0 ;

            for ( i = 1 ; i < numParametros ; i ++ ) {

            /* Obter par�metro corrente de GERATBDF */

               strcpy( ParametroCorr , vtParametros[ i ] ) ;

            /* Processar par�metro arquivo especifica��o de string */

               if ( memcmp( ParametroCorr , idParmArquivoEspec , DIM_ID_PARM ) == 0 )
               {

                  if ( numParmEspec > 0 )
                  {
                     CodigoErro = CodigoParametroDuplo ;
                     printf( "\n>>> Arquivo especifica��o %s definido multiplas vezes." , ParametroCorr ) ;
                     ContaErros ++ ;

                  } else
                  {
                     numParmEspec ++ ;
                     strcpy( NomeArquivoEspec , & ParametroCorr[ DIM_ID_PARM ] ) ;

                     if ( ObterInxExtensao( NomeArquivoEspec ) == - 1 )
                     {
                        strcat( NomeArquivoEspec , ExtArquivoEspec ) ;
                     } /* if */

                  } /* if */

               } /* fim ativa: Processar par�metro arquivo especifica��o de string */

            /* Processar par�metro tabela de defini��o a ser gerada */

               else if ( memcmp( ParametroCorr , idParmTabelaDef , DIM_ID_PARM ) == 0 )
               {

                  if ( numParmTabela > 0 )
                  {
                     CodigoErro = CodigoParametroDuplo ;
                     printf( "\n>>> Arquivo tabela %s definido multiplas vezes." , ParametroCorr ) ;
                     ContaErros ++ ;

                  } else
                  {
                     numParmTabela ++ ;
                     strcpy( NomeArquivoTabela , & ParametroCorr[ DIM_ID_PARM ] ) ;

                     if ( ObterInxExtensao( NomeArquivoTabela ) == - 1 )
                     {
                        strcat( NomeArquivoTabela , ExtArquivoTabela ) ;
                     } /* if */
                  } /* if */

               } /* fim ativa: Processar par�metro tabela de defini��o a ser gerada */

            /* Processar par�metro aux�lio de GERATBDF */

               else if ( ( memcmp( ParametroCorr , idParmAuxilio1 , DIM_ID_PARM ) == 0 )
                      || ( memcmp( ParametroCorr , idParmAuxilio2 , DIM_ID_PARM ) == 0 )
                      || ( memcmp( ParametroCorr , idParmAuxilio3 , DIM_ID_PARM ) == 0 )
                      || ( ParametroCorr[ 0 ] == '?' ))
               {

                  CodigoErro = CodigoAuxilio ;

               } /* fim ativa: Processar par�metro aux�lio de GERATBDF */

            /* Tratar par�metro errado para GERATBDF */

               else
               {

                  printf( "\n>>> Parametro errado: %s" , ParametroCorr ) ;
                  CodigoErro = CodigoErroParametro ;
                  ContaErros ++ ;

               } /* fim ativa: Tratar par�metro errado para GERATBDF */

            } /* fim repete: Processar par�metros do comando GERATBDF */

         /* Definir o nome de codigo do arquivo */

            strcpy( NomeArquivoCodigo, NomeArquivoEspec ) ;
            NomeArquivoCodigo[ ObterInxExtensao( NomeArquivoEspec ) ] = 0 ;

            tamNome = strlen( NomeArquivoCodigo ) ;
            for( i = tamNome - 1 ; i >= 0 ; i-- )
            {
               if ( NomeArquivoCodigo[ i ] == SEPARADOR_DIRETORIO )
               {
                  strcpy( &NomeArquivoCodigo[ 0 ] , &NomeArquivoCodigo[ i + 1 ] ) ;
                  break ;
               } /* if */
            } /* for */

         /* Verificar integridade de dados de GERATBDF */

            if ( numParmEspec == 0 )
            {
               CodigoErro = CodigoNaoArquivoLista ;
               printf( "\n>>> O arquivo especificacao de string nao foi definido." ) ;
               ContaErros ++ ;
            } /* if */

            if ( numParmTabela == 0 )
            {
               strcpy( NomeArquivoTabela, NomeArquivoEspec ) ;
               NomeArquivoTabela[ ObterInxExtensao( NomeArquivoEspec ) ] = 0 ;
               strcat( NomeArquivoTabela , ExtArquivoTabela ) ;
            } /* if */

         /* Listar mensagens de aux�lo da linha de comando */

            if( CodigoErro != CodigoOK )
            {

               printf( "\n\nSintaxe da linha de comando: " ) ;
               printf( "\n geratbdf { /E<Arquivo Espec> | /T<Arquivo Tabela> | /h | /H | /? | ? }" ) ;
               printf( "\n" ) ;
               printf( "\n   <Arquivo Espec> E' o nome de um arquivo que contem as" ) ;
               printf( "\n         especificacoes dos strings." ) ;
               printf( "\n   <Arquivo Tabela> parametro opcional, e' o nome do arquivo" ) ;
               printf( "\n         tabela de de definicao a ser gerado." ) ;
               printf( "\n As extensoes default sao: " ) ;
               printf( "\n   espstr - para os arquivos contendo especifica��es de strings." ) ;
               printf( "\n   tabdef - para o arquivo tabela de definicao a ser gerado." ) ;
               printf( "\n Nao sendo fornecido o nome do arquivo a ser gerado, a tabela " ) ;
               printf( "\n sera gerada no arquivo <Arquivo Espec>.tabdef" ) ;
               printf( "\n" ) ;

               if ( CodigoErro != CodigoAuxilio )
               {
                  return 4 ;
               } /* if */

               return 0 ;

            } /* fim ativa: Listar mensagens de aux�lo da linha de comando */

      /* Gerar a tabela de defini��o identificada */

         GerarTabelaDefinicao( NomeArquivoEspec , NomeArquivoTabela ) ;

      /* Terminar GERATBDF */

         if ( ContaErros > 0 )
         {
            CodigoErro = CodigoErroProcessamento ;
            printf( "\n\n>>> Foram encontrados %i erros.\n\n " ,
            ContaErros ) ;
            return 4 ;
         } /* if */

         printf( "\n\n" ) ;

         return 0 ;

   } /* Fim fun��o: GTD Processar linha de comando de gera tabela de defini��o */


/*****  C�digo das fun��es encapsuladas no m�dulo  *****/


/***********************************************************************
*
*  $FC Fun��o: GTD Gerar a tabela de defini��o
*
*  $ED Descri��o da fun��o
*     Gera a tabela de defini��o identificada nos par�metros
*
***********************************************************************/

   void GerarTabelaDefinicao( char * NomeArquivoEspec ,
                              char * NomeArquivoTabela )
   {

      FILE * pArquivoEspec  = NULL ;
      FILE * pArquivoTabela = NULL ;

      tpCondRet CondRet = CondRetErro ;

      char Buffer[     DIM_BUFFER ] ;
      char StringLido[ DIM_BUFFER ] ;

      long idString = -1 ;
      long Offset   = -1 ;

      int numLidos  = -1 ;

      /* Abrir arquivos de GERATBDF */

         pArquivoEspec  = fopen( NomeArquivoEspec  , "r" ) ;
         if ( pArquivoEspec == NULL )
         {
            printf( "\nNao abriu o arquivo de especificacao: %s" ,
                    NomeArquivoEspec ) ;
            ContaErros ++ ;
         } /* if */

         pArquivoTabela = fopen( NomeArquivoTabela , "w" ) ;
         if ( pArquivoTabela == NULL )
         {
            printf( "\nNao abriu o arquivo tabela de definicao: %s" ,
                    NomeArquivoTabela ) ;
            ContaErros ++ ;
         } /* if */

      /* Gera o texto da tabela de defini��o */

         if ( ContaErros == 0 )
         {

            /* Gerar cabe�alho da tabela de defini��o */

               fprintf( pArquivoTabela ,
                          "#ifndef _%s_str" , NomeArquivoCodigo ) ;
               fprintf( pArquivoTabela ,
                        "\n#define _%s_str" , NomeArquivoCodigo ) ;
               fprintf( pArquivoTabela ,
                        "\n" ) ;
               fprintf( pArquivoTabela ,
                        "\n/* !!!!!!!!!! Aquivo gerado! Nao o edite! !!!!!!!!!! */" ) ;
               fprintf( pArquivoTabela ,
                        "\n" ) ;
               fprintf( pArquivoTabela ,
                        "\n/****************************************************************************" ) ;
               fprintf( pArquivoTabela ,
                        "\n*" ) ;
               fprintf( pArquivoTabela ,
                        "\n* $AT Tabela de defini��o de chaves de acesso a strings gerada" ) ;
               fprintf( pArquivoTabela ,
                        "\n*" ) ;
               fprintf( pArquivoTabela ,
                        "\n* Arquivo gerado: %s" , NomeArquivoTabela ) ;
               fprintf( pArquivoTabela ,
                        "\n*" ) ;
               fprintf( pArquivoTabela ,
                        "\n* Projeto: INF 1301/1628 Exemplos" ) ;
               fprintf( pArquivoTabela ,
                        "\n* Gestor:  LES/DI/PUC-Rio" ) ;
               fprintf( pArquivoTabela ,
                        "\n*" ) ;
               fprintf( pArquivoTabela ,
                        "\n* Arquivo de especifica��o de strings: %s" , NomeArquivoEspec ) ;
               fprintf( pArquivoTabela ,
                        "\n*" ) ;
               fprintf( pArquivoTabela ,
                        "\n***************************************************************************/" ) ;
               fprintf( pArquivoTabela ,
                        "\n" ) ;

            /* Obter offset das defini��es */

               CondRet = LerTrimmed( pArquivoEspec , Buffer , DIM_BUFFER - 1 ,
                                     LIMPA_INIC | LIMPA_FIM ) ;

               if ( CondRet != CondRetOK )
               {
                  printf( "\nArquivo %s vazio" , NomeArquivoEspec ) ;
                  ContaErros ++ ;
               } else
               {
                  numLidos = sscanf( Buffer , " %s %li" , StringLido , &Offset ) ;
                  if ( ( numLidos != 2 )
                    || ( strcmp( StringLido , CMD_OFFSET ) != 0 ))
                  {
                     printf( "\nFaltou a defini��o de offset em %s" , NomeArquivoEspec ) ;
                     ContaErros ++ ;
                  } /* if */
               } /* if */

            /* Processar as especifica��es */

               if ( ContaErros == 0 )
               {

                  CondRet = LerTrimmed( pArquivoEspec , Buffer , DIM_BUFFER - 1 ,
                                        LIMPA_INIC | LIMPA_FIM ) ;

                  while ( CondRet == CondRetOK )
                  {
                     numLidos = sscanf( Buffer , " %s %li" , StringLido , &idString ) ;
                     if ( numLidos != 2 )
                     {
                        printf( "\nArquivo de especifica��o errado: %s" , Buffer ) ;
                        ContaErros ++ ;
                        break ;
                     } /* if */

                     fprintf( pArquivoTabela ,
                              "\n   const int %-20s = %5li ;" ,
                              StringLido , idString + Offset ) ;
                     CondRet = LerTrimmed( pArquivoEspec , Buffer , DIM_BUFFER - 1 ,
                                           LIMPA_INIC | LIMPA_FIM ) ;

                  } /* while */

               } /* fim ativa: Processar as especifica��es */

            /* Gerar t�rmino do arquivo */

               fprintf( pArquivoTabela ,
                        "\n" ) ;
               fprintf( pArquivoTabela ,
                        "\n#endif" ) ;
               fprintf( pArquivoTabela ,
                        "\n" ) ;
               fprintf( pArquivoTabela ,
                        "\n/* Fim tabela de defini��o gerada */" ) ;
               fprintf( pArquivoTabela ,
                        "\n" ) ;

         } /* fim ativa: Gera o texto da tabela de defini��o */

      /* Fechar arquivos de GERATBDF */

         if ( pArquivoEspec != NULL )
         {
            fclose( pArquivoEspec ) ;
         } /* if */

         if ( pArquivoTabela != NULL )
         {
            fclose( pArquivoTabela ) ;
         } /* if */

   } /* Fim fun��o: GTD Gerar a tabela de defini��o */


/***********************************************************************
*
*  $FC Fun��o: GT Ler eliminando brancos no in�cio e no final
*
*  $ED Descri��o da fun��o
*     L� uma linha, eliminando os caracteres "branco" no in�cio e no final
*     Salta linhas de coment�rio
*     Linhas nulas ap�s elimina��o de brancos s�o saltadas
*
*  $EP Par�metros
*     pArqLe     - ponteiro para descritor de arquivo de leitura
*     pBuffer    - ponteiro para a �rea de leitura
*     dimBuffer  - dimens�o do buffer
*     Modo       - modo de limpeza, qualquer combina��o ( OR ) de 0,1 ou 2
*                  das condi��es a seguir :
*                      LIMPA_INIC - limpa antes
*                      LIMPA_FIM  - limpa no final
*
*  $FV Valor retornado
*     CondRetOK     - leu um buffer v�lido
*
*     CondRetFimArq - encontrou fim de arquivo
*                     Buffer cont�m string nulo
*
*     CondRetErro   - erro permanente de leitura
*                     Buffer cont�m string nulo
*
***********************************************************************/

   tpCondRet LerTrimmed( FILE * pArqLe    ,
                         char * pBuffer   ,
                         int    dimBuffer ,
                         int    Modo       )
   {

      int Continua = 0 ;
      int i        = 0 ;

      int tamLinha ;

      /* Controlar final de arquivo */

         if ( feof( pArqLe ))
         {

            pBuffer[ 0 ] = 0 ;
            return CondRetFimArq ;

         } /* fim ativa: Controlar final de arquivo */

      /* Procurar linha n�o vazia */

         Continua = 1 ;

         while ( Continua ) {

         /* Ler linha de arquivo */

            pBuffer[ dimBuffer - 1 ] = '@' ;

            if ( fgets( pBuffer , dimBuffer , pArqLe ) == NULL )
            {
               pBuffer[ 0 ] = 0 ;
               if ( feof( pArqLe ))
               {
                  return CondRetFimArq ;
               } else {
                  return CondRetErro ;
               } /* if */
            } /* if */

            if ( pBuffer[ dimBuffer - 1 ] != '@' )
            {
               printf( "\n>>> Linha longa demais" ) ;
               ContaErros ++ ;
               return CondRetOverflow ;
            } /* if */

         /* Limpar a linha */

            /* Eliminar lixo no final */

               if ( Modo & LIMPA_FIM )
               {

                  tamLinha = ( int ) strlen( pBuffer ) ;
                  for( i = tamLinha - 1 ; i >= 0 ; i -- )
                  {
                     if ( strchr( CHARS_FIM , pBuffer[ i ] ) == NULL )
                     {
                        break ;
                     } /* if */
                  } /* for */

                  i++ ;
                  pBuffer[ i ] = 0 ;

               } /* fim ativa: Eliminar lixo no final */

            /* Eliminar lixo no in�cio */

               if ( Modo & LIMPA_INIC )
               {

                  tamLinha = ( int ) strlen( pBuffer ) ;
                  for( i = 0 ; i < tamLinha ; i++ )
                  {
                     if ( pBuffer[ i ] != ' ' )
                     {
                        break ;
                     } /* if */
                  } /* for */

                  if ( i > 0 )
                  {
                     strcpy( pBuffer , pBuffer + i ) ;
                  } /* if */

               } /* fim ativa: Eliminar lixo no in�cio */

         /* Determinar o que fazer com a linha */

            Continua = 0 ;

            if ( ( memcmp( pBuffer , ID_COMENTARIO , strlen( ID_COMENTARIO )) == 0 )
              || ( strlen( pBuffer ) == 0 ))
            {
               Continua = 1 ;
            } /* if */

         } /* fim repete: Procurar linha n�o vazia */

      return CondRetOK ;

   } /* Fim fun��o: GT Ler eliminando brancos no in�cio e no final */


/***********************************************************************
*
*  $FC Fun��o: GT Obter �ndice do nome de extens�o
*
*  $EP Par�metros
*     NomeArquivo - nome do arquivo a examinar
*
*  $FV Valor retornado
*     Indice do caractere '.' separador do nome de extens�o
*        ser� -1 caso o nome de extens�o n�o exista
*
***********************************************************************/

   int ObterInxExtensao( char * NomeArquivo )
   {

      int Encontrou ,
          i ;

      Encontrou = -1 ;

      for( i = strlen( NomeArquivo ) - 1 ; i > 0  ; i -- )
      {
         if ( NomeArquivo[ i ] == SEPARADOR_EXTENSAO )
         {
            Encontrou = i ;
            break ;

         } else if ( NomeArquivo[ i ] == SEPARADOR_DIRETORIO )
         {
            break ;

         } /* if */
      } /* for */

      return Encontrou ;

   } /* Fim fun��o: GT Obter �ndice do nome de extens�o */

/********** Fim do m�dulo de implementa��o: GTD  Gerar tabela de defini��o **********/

