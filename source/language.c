#include "language.h"

/*
	The languages are in the order based on CFG_Language:
	
	0. Japanese
	1. English
	2. French
	3. German
	4. Italian
	5. Spanish
	6. Simplified Chinese
	7. Korean
	8. Dutch
	9. Portugese 
	10. Russian
	11. Traditional Chinese
*/

const char lang_welcome[][3][100] = 
{	
	{"こんにちは", "ご機嫌よう", "今日は"},
	{"Hello there", "How are you today?", "Today is"},
	{"Allo", "Comment allez-vous aujourd'hui?", "Aujourd'hui, c'est"},
	{"Hallo", "Wie geht es dir heute?", "Heute ist"},
	{"Ciao", "Come stai?", "Oggi è"},
	{"Hola", "¿Cómo estás?", "Hoy es"},
	{"你好！", "今天过得怎么样？", "今天是"},
	{"안녕하세요!", "오늘 하루는 어떠세요?", "오늘은 "},
	{"Hallo", "Hoe gaat het?", "Vandaag is het"},
	{"Olá!", "Como está hoje?", "Hoje é"},
	{"Привет", "Как поживаешь?", "Сегодня"},
	{"嗨", "今天好嗎?", "今天是"}
};

const char lang_months[][12][50] = 
{	
	{"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"},
	{"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"},
	{"Janvier", "Février", "Mars", "Avril", "Mai", "Juin", "Juillet", "Août", "Septembre", "Octobre", "Novembre", "Décembre"},
	{"Januar", "Februar", "März", "April", "Mai", "Juni", "Juli", "August", "September", "Oktober", "November", "Dezember"},
	{"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"},
	{"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"},
	{"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"},
	{"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"},
	{"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"},
	{"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"},
	{"Январь", "Февраль", "Март", "Апрель", "Май", "Июнь", "Июль", "Август", "Сентябрь", "Октябрь", "Ноябрь", "Декабрь"},
	{"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"}
};

const char lang_days[][7][50] = 
{	
	{"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"},
	{"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"},
	{"Dimanche", "Lundi", "Mardi", "Mercredi", "Jeudi", "Vendredi", "Samedi"},
	{"Sonntag", "Montag", "Dienstag", "Mittwoch", "Donnerstag", "Freitag", "Samstag"},
	{"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"},
	{"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"},
	{"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"},
	{"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"},
	{"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"},
	{"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"},
	{"Воскресенье", "Понедельник", "Вторник", "Среда", "Четверг", "Пятница", "Суббота"},
	{"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"}
};

const char lang_files[][11][50] = 
{	
	{"Parent folder", "Folder", "APP", "Payload", "archive/zip", "archive/rar", "image/png", "image/jpeg", "audio/mpeg", "TEXT", "FILE"},
	{"Parent folder", "Folder", "APP", "Payload", "archive/zip", "archive/rar", "image/png", "image/jpeg", "audio/mpeg", "TEXT", "FILE"},
	{"Parent folder", "Folder", "APP", "Payload", "archive/zip", "archive/rar", "image/png", "image/jpeg", "audio/mpeg", "TEXT", "FILE"},
	{"Parent folder", "Folder", "APP", "Payload", "archive/zip", "archive/rar", "image/png", "image/jpeg", "audio/mpeg", "TEXT", "FILE"},
	{"Parent folder", "Folder", "APP", "Payload", "archive/zip", "archive/rar", "image/png", "image/jpeg", "audio/mpeg", "TEXT", "FILE"},
	{"Parent folder", "Folder", "APP", "Payload", "archive/zip", "archive/rar", "image/png", "image/jpeg", "audio/mpeg", "TEXT", "FILE"},
	{"Parent folder", "Folder", "APP", "Payload", "archive/zip", "archive/rar", "image/png", "image/jpeg", "audio/mpeg", "TEXT", "FILE"},
	{"Parent folder", "Folder", "APP", "Payload", "archive/zip", "archive/rar", "image/png", "image/jpeg", "audio/mpeg", "TEXT", "FILE"},
	{"Parent folder", "Folder", "APP", "Payload", "archive/zip", "archive/rar", "image/png", "image/jpeg", "audio/mpeg", "TEXT", "FILE"},
	{"Parent folder", "Folder", "APP", "Payload", "archive/zip", "archive/rar", "image/png", "image/jpeg", "audio/mpeg", "TEXT", "FILE"},
	{"Parent folder", "Folder", "APP", "Payload", "archive/zip", "archive/rar", "image/png", "image/jpeg", "audio/mpeg", "TEXT", "FILE"},
	{"Parent folder", "Folder", "APP", "Payload", "archive/zip", "archive/rar", "image/png", "image/jpeg", "audio/mpeg", "TEXT", "FILE"}
};

const char lang_options[][9][100] =
{
	{"アクション", "プロパティ", "名前を変更", "フォルダを作る", "コピー", "削除", "切り取る", "Paste", "キャンセル"},
	{"Actions", "Properties", "Rename", "New folder", "Copy", "Delete", "Cut", "Paste", "CANCEL"},
	{"Actions", "Propriétés", "Renommer", "Nouveau dossier", "Copier", "Supprimer", "Couper", "Paste", "ANNULER"},
	{"Aktionen", "Eigenschaften", "Umbenennen", "Neuer Ordner", "Kopieren", "Löschen", "Ausschneiden", "Paste", "ABBRECHEN"},
	{"Azioni", "Proprietà", "Rinomina", "Nuova cartella", "Copia", "Elimina", "Taglia", "Paste", "ANNULLA"},
	{"Acciones", "Propiedades", "Renombrar", "Nueva carpeta", "Copiar", "Eliminar", "Cortar", "Paste", "CANCELAR"},
	{"操作", "属性", "重命名", "新建文件夹", "复制", "删除", "剪切", "Paste", "取消"},
	{"작업", "속성", "이름 바꾸기", "새 폴더", "복사", "삭제", "잘라내기", "Paste", "취소"},
	{"Actie's", "Instellingen", "Naam wijzigen", "Nieuwe map", "Kopiëren", "Verwijderen", "Knippen", "Paste", "ANNULEREN"},
	{"Ações", "Propriedades", "Renomear", "Nova pasta", "Copiar", "Eliminar", "Recortar", "Paste", "CANCELAR"},
	{"Действия", "Свойства", "Переименовать", "Новая папка", "Копировать", "Удалить", "Вырезать", "Paste", "Отменить"},
	{"動作", "屬性", "重新命名", "新資料夾", "複製", "刪除", "剪下", "Paste", "取消"}
};

const char lang_properties[][7][100] =
{
	{"プロパティ", "詳細", "名前", "親ディレクトリ", "タイプ", "Size", "OK"},
	{"Properties", "INFO", "Name", "Parent", "Type", "Size", "OK"},
	{"Propriétés", "INFO", "Nom", "Parent", "Type", "Size", "OK"},
	{"Eigenschaften", "INFO", "Name", "Übergeordnet", "Typ", "Size", "OK"},
	{"Proprietà", "INFO", "Nome", "Cartella", "Tipo", "Size", "OK"},
	{"Propiedades", "Información", "Nombre", "Padre", "Tipo", "Size", "Confirmar"},
	{"属性", "信息", "名称", "父目录", "类型", "Size", "OK"},
	{"속성", "정보", "이름", "부모", "타입", "Size", "확인"},
	{"Instellingen", "INFO", "Naam", "Omhoog", "Soort", "Size", "Oke"},
	{"Propriedades", "INFO", "Nome", "Local", "Tipo", "Size", "OK"},
	{"Свойства", "ИНФО", "Название", "Вверх", "Тип", "Size", "OK"},
	{"屬性", "資訊", "名字", "根目錄", "型態", "Size", "OK"}
};

const char lang_deletion[][5][100] =
{
	{"削除を確認", "削除されたファイルは引き戻せません", "削除しますか", "いいえ", "はい"},
	{"Confirm deletion", "This action cannot be undone.", "Do you want to continue", "NO", "YES"},
	{"Confirmer la supprimation", "Cette action est irréversible.", "Voulez-vous continuer", "NON", "OUI"},
	{"Löschen bestätigen", "Diese Aktion kann nicht rückgängig gemacht werden.", "Fortsetzen?", "NEIN", "JA"},
	{"Conferma eliminazione", "Questa azione non può essere ripristinata.", "Vuoi continuare", "NO", "SI"},
	{"Confirmar borrado", "Esta acción no puede ser revertida", "¿Desea continuar?", "NO", "SÍ"},
	{"确认删除", "该操作结果不可逆", "是否确认继续？", "取消", "确认"},
	{"삭제 확인", "삭제한 파일은 되돌릴 수 없습니다.", "계속 할까요?", "아니요", "예"},
	{"Bevestig Verwijdering", "Deze actie kan niet ongedaan worden gemaakt.", "Weet u het zeker?", "NEE(N)", "JA"},
	{"Eliminar arquivo", "Esta ação não pode ser anulada.", "Deseja continuar?", "NÃO", "SIM"},
	{"Подтвердите удаление", "Вы не сможете восстановить удаленные файлы.", "Хотите продолжить", "НЕТ", "ДА"},
	{"確認刪除", "這個動作無法復原", "你希望繼續嗎", "否", "是"}
};

const char lang_settings[][9][100] =
{
	{"一般的", "システムファイル防御", "大切なファイルの削除を防止します", "カスタムテーマ", "現在のテーマ:", "Recycle Bin", "Enable 3D-Shell's recyle bin."},
	{"General", "Display filename in Gallery", "Toggles filename display in Gallery.", "Custom themes", "Current:", "Recycle Bin", "Enable 3D-Shell's recyle bin.", "Show hidden files", "Displays hidden files in file browser."},
	{"Général", "Protection de fichier système", "Empêche la supprimation des fichiers systèmes.", "Thèmes personnalisés", "Actuel:", "Recycle Bin", "Enable 3D-Shell's recyle bin."},
	{"Allgemein", "Systemdateien-Schutz", "Verhindert das Löschen von Systemdateien.", "Eigene Themen", "Aktuell:", "Recycle Bin", "Enable 3D-Shell's recyle bin."},
	{"Generale", "Protezione file di sistema", "Previene l'eliminazione di file di sistema.", "Temi custom", "Attuale:", "Recycle Bin", "Enable 3D-Shell's recyle bin."},
	{"General", "Protección de archivos del sistema", "Previene la eliminación de archivos del sistema", "Temas personalizados", "Actual:", "Recycle Bin", "Enable 3D-Shell's recyle bin."},
	{"通用", "系统文件保护", "防止系统文件被删除。", "自定义主题", "当前：", "Recycle Bin", "Enable 3D-Shell's recyle bin."},
	{"일반", "시스템 파일 보호", "시스템 파일의 삭제를 방지합니다.", "커스텀 테마", "현재:", "Recycle Bin", "Enable 3D-Shell's recyle bin."},
	{"Algemeen", "System file protection", "Voorkomt het verwijderen van systeembestanden.", "Aangepaste Thema's", "Momenteel:", "Recycle Bin", "Enable 3D-Shell's recyle bin."},
	{"Geral", "Proteção de arquivos do sistema", "Prevenir eliminação de arquivos do sistema", "Customização", "Tema atual:", "Recycle Bin", "Enable 3D-Shell's recyle bin."},
	{"Общее", "Защита файловой системы", "Предотвращать удаление системных файлов.", "Темы", "Текущая:", "Recycle Bin", "Enable 3D-Shell's recyle bin."},
	{"一般", "系統資料保護", "保護系統資料不被刪除", "自製主題", "當前:", "Recycle Bin", "Enable 3D-Shell's recyle bin."}
};

const char lang_themes[][1][100] =
{
	{"テーマを選択してください"},
	{"Select a theme"},
	{"Choisissez un thème"},
	{"Thema auswählen"},
	{"Seleziona un tema"},
	{"Selecciona un tema"},
	{"选择一个主题"},
	{"테마 선택"},
	{"Selecteer een thema"},
	{"Selecione um tema"},
	{"Выберите тему"},
	{"請選擇一個主題"}
};

const char lang_ftp[][5][100] = 
{
	{"FTP起動しました", "この状態ではブラウザが使えません", "FTPアイコンをタッチすると、FTP接続が終了します", "Failed to initialize FTP.", "WiFi not enabled."},
	{"FTP initialized", "File browser cannot be accesed at this time.", "Tap the FTP icon to disable the FTP connection.", "Failed to initialize FTP.", "WiFi not enabled."},
	{"FTP initialisé", "Le navigateur de fichiers ne peut pas être accédé en ce moment.", "Appuyer sur l'icône FTP pour désactiver la connection de FTP.", "Failed to initialize FTP.", "WiFi not enabled."},
	{"FTP gestartet", "Auf den Dateibrowser kann zur Zeit nicht zugegriffen werden.", "Wähle das FTP-Icon aus, um die FTP-Verbindung zu deaktivieren.", "Failed to initialize FTP.", "WiFi not enabled."},
	{"Avviato FTP", "Non è possibile accedere al momento al browser di file.", "Tocca l'icona FTP per disattivare la connessione FTP.", "Failed to initialize FTP.", "WiFi not enabled."},
	{"FTP inicializado", "El explorador de archivos no puede ser utilizado actualmente.", "Toca el ícono de FTP para desactivar la conexión FTP.", "Failed to initialize FTP.", "WiFi not enabled."},
	{"FTP 初始化完毕", "文件浏览器当前不可用。", "点击FTP图标以禁用FTP连接。", "Failed to initialize FTP.", "WiFi not enabled."},
	{"FTP 초기화 완료", "현재는 파일 브라우저를 이용할 수 없습니다.", "FTP 아이콘을 탭해 FTP를 비활성화", "Failed to initialize FTP.", "WiFi not enabled."},
	{"FTP gestart", "Bestandsbeheer kan niet worden geopend op dit moment.", "Klik op de FTP icoon om de FTP service uit te zetten.", "Failed to initialize FTP.", "WiFi not enabled."},
	{"FPT inicializado", "O explorador de arquivos não poderá ser utilizado.", "Toque no ícone FTP para terminar a conexão.", "Failed to initialize FTP.", "WiFi not enabled."},
	{"FTP запущен", "В настоящий момент файловый браузер недоступен.", "Нажмите на иконку FTP для прерывания FTP соединения.", "Failed to initialize FTP.", "WiFi not enabled."},
	{"FTP初始化中", "目前無法取得檔案管理員的資料", "請點選FTP的按鈕來結束與FTP的連線", "Failed to initialize FTP.", "WiFi not enabled."}
};

const char lang_update[][4][100] = 
{
	{"Checking for updates...", "Install failed", "Install success", "Exiting.."},
	{"Checking for updates...", "Install failed", "Install success", "Exiting.."},
	{"Vérification des mises à jour...", "Installation échouée", "Installation réussie", "Fermeture..."},
	{"Suche nach Updates...", "Installation fehlgeschlagen", "Installation erfolgreich", "Beenden..."},
	{"Checking for updates...", "Install failed", "Install success", "Exiting.."},
	{"Checking for updates...", "Install failed", "Install success", "Exiting.."},
	{"Checking for updates...", "Install failed", "Install success", "Exiting.."},
	{"Checking for updates...", "Install failed", "Install success", "Exiting.."},
	{"Checking for updates...", "Install failed", "Install success", "Exiting.."},
	{"Checking for updates...", "Install failed", "Install success", "Exiting.."},
	{"Проверка обновлений...", "Установка не удалась", "Успешная установка", "Выходим..."},
	{"Checking for updates...", "Install failed", "Install success", "Exiting.."}
};