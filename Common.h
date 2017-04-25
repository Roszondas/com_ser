#define	TIMEOUT			1

#define COM_HNDSHAKE	"787"
#define COM_READY		"325"
#define COM_END			"842"

enum {INTRFCE_COM, INTRFCE_TCP};
enum port_state {WAITING, HNDSHAKED, READING};
