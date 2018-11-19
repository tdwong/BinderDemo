/* -*- mode: C++; c-basic-offset: 4; indent-tabs-mode: nil -*-
   vi:ai:tabstop=8:shiftwidth=4:softtabstop=4:expandtab
*/

/*
 * Author: Gabriel Burca <gburca dash binder at ebixio dot com>
 *
 * Sample code for using binders in Android from C++
 *
 * The Demo service provides 3 operations: push(), alert(), add(). See
 * the IDemo class documentation to see what they do.
 *
 * Both the server and client code are included below.
 *
 * To view the log output:
 *      adb logcat -v time binder_demo:* *:S

//
//	$ mldb logcat -v time -s binder_demo:*
//

 *
 * To run, create 2 adb shell sessions. In the first one run "binder" with no
 * arguments to start the service. In the second one run "binder N" where N is
 * an integer, to start a client that connects to the service and calls push(N),
 * alert(), and add(N, 5).
 */

#define LOG_TAG "binder_demo"

#define SERVICE_NAME "binder_Demo"

//
// NOTE:
//	- define START_CLIENT_THREAD to use a separate thread for remote operation(s)
//	- linkToDeath must be registered in the same thread that performs remote operation
//
#define	START_CLIENT_THREAD		1	// run a separate thread
#if	defined(START_CLIENT_THREAD)
	#define	LINK_TO_DEATH_IN_CLIENT	1
#else
	#undef	LINK_TO_DEATH_IN_CLIENT
#endif	//!START_CLIENT_THREAD

//
// enable one or more, or none remote operation(s) to be performed in the loop
//
//#define	REMOTE_ALERT	0x01
//#define	REMOTE_PUSH		0x02
#define	REMOTE_ADD		0x04

/* For relevant code see:
    frameworks/native/{include,libs}/binder/{IInterface,Parcel}.{h,cpp}
    system/core/include/utils/{Errors,RefBase}.h
 */

#include <stdlib.h>
#include <pthread.h>

#include <utils/RefBase.h>
#include <utils/Log.h>
#include <binder/TextOutput.h>

#include <binder/IInterface.h>
#include <binder/IBinder.h>
#include <binder/ProcessState.h>
#include <binder/IServiceManager.h>
#include <binder/IPCThreadState.h>
#include <binder/Status.h>

using namespace android;


#define INFO(...) \
    do { \
        printf(__VA_ARGS__); \
        printf("\n"); \
        ALOGD(__VA_ARGS__); \
    } while(0)

void assert_fail(const char *file, int line, const char *func, const char *expr) {
    INFO("assertion failed at file %s, line %d, function %s:",
            file, line, func);
    INFO("%s", expr);
    abort();
}

#define ASSERT(e) \
    do { \
        if (!(e)) \
            assert_fail(__FILE__, __LINE__, __func__, #e); \
    } while(0)


// Where to print the parcel contents: aout, alog, aerr. alog doesn't seem to work.
#define PLOG aout



// Interface (our AIDL) - Shared by server and client
class IDemo : public IInterface
{
    public:
        enum {
            ALERT = IBinder::FIRST_CALL_TRANSACTION,
            PUSH,
            ADD
        };
        // Sends a user-provided value to the service
        virtual void        push(int32_t data)          = 0;
        // Sends a fixed alert string to the service
        virtual void        alert()                     = 0;
        // Requests the service to perform an addition and return the result
        virtual int32_t     add(int32_t v1, int32_t v2) = 0;

	//
	// see ${NOVA}/frameworks/native/include/binder/IInterface.h
	//
        DECLARE_META_INTERFACE(Demo);  // Expands to 5 lines below:
        //static const android::String16 descriptor;
        //static android::sp<IDemo> asInterface(const android::sp<android::IBinder>& obj);
        //virtual const android::String16& getInterfaceDescriptor() const;
        //IDemo();
        //virtual ~IDemo();
};	// class IDemo : public IInterface

// Client
class BpDemo : public BpInterface<IDemo>
{
    public:
        BpDemo(const sp<IBinder>& impl) : BpInterface<IDemo>(impl) {
            ALOGD("BpDemo::BpDemo()");
        }

        virtual void push(int32_t push_data) {
            Parcel data, reply;
            data.writeInterfaceToken(IDemo::getInterfaceDescriptor());
            data.writeInt32(push_data);

            aout << "BpDemo::push parcel to be sent:\n";
            data.print(PLOG); endl(PLOG);

            remote()->transact(PUSH, data, &reply);

            aout << "BpDemo::push parcel reply:\n";
            reply.print(PLOG); endl(PLOG);

            ALOGD("BpDemo::push(%i)", push_data);
        }

        virtual void alert() {
            Parcel data, reply;
            data.writeInterfaceToken(IDemo::getInterfaceDescriptor());
            data.writeString16(String16("The alert string"));
            remote()->transact(ALERT, data, &reply, IBinder::FLAG_ONEWAY);    // asynchronous call
            ALOGD("BpDemo::alert()");
        }

        virtual int32_t add(int32_t v1, int32_t v2) {
            Parcel data, reply;
            data.writeInterfaceToken(IDemo::getInterfaceDescriptor());
            data.writeInt32(v1);
            data.writeInt32(v2);
            aout << "BpDemo::add parcel to be sent:\n";
              ALOGD("BpDemo::add parcel to be sent:");		// send to log
            data.print(PLOG); endl(PLOG);
            remote()->transact(ADD, data, &reply);
            aout << "BpDemo::add transact reply\n";			// send to stdou
              ALOGD("BpDemo::add transact reply");
            reply.print(PLOG); endl(PLOG);

            int32_t res = 0;
            status_t status = reply.readInt32(&res);
			// Status s=Status::fromStatusT(status).toString8().string();
            aout << "BpDemo::add transact reply\n";			// send to stdou
            printf("BpDemo::add(%i, %i) = %i (status: %i: %s)\n", v1, v2, res, status, android::binder::Status::fromStatusT(status).toString8().string());
             ALOGD("BpDemo::add(%i, %i) = %i (status: %i: %s)", v1, v2, res, status, android::binder::Status::fromStatusT(status).toString8().string());
            return res;
        }
};	// class BpDemo : public BpInterface<IDemo>

//
// see ${NOVA}/frameworks/native/include/binder/IInterface.h
//
    //IMPLEMENT_META_INTERFACE(Demo, "Demo");
    // Macro above expands to code below. Doing it by hand so we can log ctor and destructor calls.
    const android::String16 IDemo::descriptor(SERVICE_NAME);
    const android::String16& IDemo::getInterfaceDescriptor() const {
        return IDemo::descriptor;
    }
    android::sp<IDemo> IDemo::asInterface(const android::sp<android::IBinder>& obj) {
        android::sp<IDemo> intr;
        if (obj != NULL) {
            intr = static_cast<IDemo*>(obj->queryLocalInterface(IDemo::descriptor).get());
            if (intr == NULL) {
                intr = new BpDemo(obj);
            }
        }
        return intr;
    }
    IDemo::IDemo() { ALOGD("IDemo::IDemo()"); }
    IDemo::~IDemo() { ALOGD("IDemo::~IDemo()"); }
    // End of macro expansion

// Server
class BnDemo : public BnInterface<IDemo>
{
    virtual status_t onTransact(uint32_t code, const Parcel& data, Parcel* reply, uint32_t flags = 0);
};

//
// see ${NOVA}/frameworks/native/include/binder/Parcel.h
//
status_t BnDemo::onTransact(uint32_t code, const Parcel& data, Parcel* reply, uint32_t flags)
{
    ALOGD("BnDemo::onTransact(%i) %i", code, flags);
    data.checkInterface(this);
    data.print(PLOG); endl(PLOG);

    switch(code) {
        case ALERT: {
			android::String16 str;
			android::status_t rc = data.readString16(&str);
											///https://codingnote.blogspot.com/2017/02/c-print-string16-in-android.html
            ALOGD("BnDemo::onTransact got \"%s\"", String8(str).string());
            alert();    // Ignoring the fixed alert string
            return NO_ERROR;
        } break;
        case PUSH: {
            int32_t inData = data.readInt32();
            ALOGD("BnDemo::onTransact got %i", inData);
            push(inData);
            ASSERT(reply != 0);
            reply->print(PLOG); endl(PLOG);
            return NO_ERROR;
        } break;
        case ADD: {
            int32_t inV1 = data.readInt32();
            int32_t inV2 = data.readInt32();
            int32_t sum = add(inV1, inV2);
            ALOGD("BnDemo::onTransact add(%i, %i) = %i", inV1, inV2, sum);
            ASSERT(reply != 0);
            reply->print(PLOG); endl(PLOG);
            reply->writeInt32(sum);
            return NO_ERROR;
        } break;
        default:
            return BBinder::onTransact(code, data, reply, flags);
    }
}

class Demo : public BnDemo
{
    virtual void push(int32_t data) {
        INFO("Demo::push(%i)", data);
    }
    virtual void alert() {
        INFO("Demo::alert()");
    }
    virtual int32_t add(int32_t v1, int32_t v2) {
        INFO("Demo::add(%i, %i)", v1, v2);
        return v1 + v2;
    }
};


/*
// https://www.androiddesignpatterns.com/2013/08/binders-death-recipients.html
//
//	Death Recipients
//
//	For system/service to detact an application's death so it can quickly clean up its state ---
//	this task is made easy using the Binder’s “link-to-death” facility, which allows a process to get a callback
//	when another process hosting a binder object goes away. In Android, any process can receive a notification
//	when another process dies by taking the following steps:
//
//	* First, the process creates a DeathRecipient callback object containing the code to be executed
//	  when the death notification arrives.
//
//	* Next, it obtains a reference to a Binder object that lives in another process and calls its
//	  linkToDeath(IBinder.DeathRecipient recipient, int flags), passing the DeathRecipient callback object
//	  as the first argument.
//
//	* Finally, it waits for the process hosting the Binder object to die. When the Binder kernel driver detects
//	  that the process hosting the Binder is gone, it will notify the registered DeathRecipient callback object
//	  by calling its binderDied() method.
//
*/

//
// see ${NOVA}/frameworks/native/include/binder/IBinder.h: class DeathRecipient : public virtual RefBase
//
/*
    class DeathRecipient : public virtual RefBase
    {
    public:
        virtual void binderDied(const wp<IBinder>& who) = 0;
    };
*/
//
bool gClientRun = true;
class DeathNotifier : public android::IBinder::DeathRecipient
{
	public:
		DeathNotifier() {}

        virtual void binderDied(const wp<IBinder>& /*who*/) override {
			// it appears that binderDied() is runing in the main() thread context
			printf("-->>> %s: thread id:%lx...\n",__func__,pthread_self());
			 INFO("death notification: %s", __func__);
			ALOGD("death notification: %s", __func__);

			// stop clnt() thread
			gClientRun = false;

			// stop main thread
			android::IPCThreadState::self()->stopProcess();

#if	0	// intentional client crash -> if use Listener, service provider will be notified
		/*
		// intentional NULL pointer de-reference
			int *p = NULL;
			*p = 1;
		*/
#endif	//0
		}
};


// Helper function to get a hold of the "Demo" service.
///sp<IDemo> getDemoServ()
sp<IBinder> getDemoServ()
{
//
// see ${NOVA}/frameworks/native/include/binder/IServiceManager.h:sp<IServiceManager> defaultServiceManager();
//
    sp<IServiceManager> sm = defaultServiceManager();
    ASSERT(sm != 0);
    sp<IBinder> binder = sm->getService(String16(SERVICE_NAME));
    // TODO: If the "Demo" service is not running, getService times out and binder == 0.
    ASSERT(binder != 0);

		status_t rc = binder->pingBinder();
		INFO("pingBinder: rc=%d", rc);

	//
	// do NOT register linkToDeath() here. No death notification will be triggered as the DeathNotifier() is out-of-scope when getDemoServ() returns
	//
///    sp<IDemo> demo = interface_cast<IDemo>(binder);
///    ASSERT(demo != 0);
///    return demo;
	// return binder sp (so we can do linkToDeath() in main thread
    return binder;
}


// struct to pass to clnt() thread
// using struct to preserve android::sp
struct thread_data {
	sp<IDemo>   spIDemo;
	sp<IBinder> spIBinder;
};

// client function/thread
void *clnt(void *ptr)
{
	printf("-->>> %s: thread id:%lx...\n",__func__,pthread_self());

	thread_data *p = (thread_data*) ptr;
	sp<IDemo> demo = p->spIDemo;

#if	defined(LINK_TO_DEATH_IN_CLIENT)
	sp<IBinder> binder = p->spIBinder;

		sp<DeathNotifier> spDeathNotifier = new DeathNotifier();
		status_t rc = binder->linkToDeath(spDeathNotifier);
		INFO("linkToDeath: rc=%d (tid=%lx", rc, pthread_self());
#endif	//LINK_TO_DEATH_IN_CLIENT

	while (gClientRun) {

		/*
		// will receive death notification even with simple DO-nothing loop
		*/

	#if	defined(REMOTE_ADD)
		// will receive death notification with add()
		int32_t sum = demo->add(2, 3);
		ALOGD("demo->add(), sum=%d", sum);
	#endif	//REMOTE_ADD

	#if	defined(REMOTE_PUSH)
		// will receive death notification with push()
		demo->push(23);
		ALOGD("demo->push()");
	#endif	//REMOTE_PUSH

	#if	defined(REMOTE_ALERT)
		// will receive death notification with push()
		demo->alert();
		ALOGD("demo->alert()");
	#endif	//REMOTE_ALERT

		sleep(3);
	}

	ALOGD("clnt exiting... gClientRun=%d",gClientRun);

	return NULL;
}

int main(int argc, char **argv) {

    if (argc == 1) {
        ALOGD("We're the service");

		// register SERVICE_NAME as my service with Context Manager
        defaultServiceManager()->addService(String16(SERVICE_NAME), new Demo());
		// kickstart the server
        android::ProcessState::self()->startThreadPool();
        ALOGD("Binder_Demo service is now ready");
        IPCThreadState::self()->joinThreadPool();
        ALOGD("Binder_Demo service thread joined");

		//
		// if we ever get out of joinThreadPool() via IPCThreadState::self()->stopProcess();
		//
		android::IPCThreadState::shutdown();

    } else if (argc == 2) {
        INFO("We're the client: %s", argv[1]);

        int v = atoi(argv[1]);

///        sp<IDemo> demo = getDemoServ();
        sp<IBinder> binder = getDemoServ();
        sp<IDemo> demo = interface_cast<IDemo>(binder);
        ASSERT(demo != 0);

        demo->alert();
        demo->push(v);
        const int32_t adder = 5;
        int32_t sum = demo->add(v, adder);
        ALOGD("Addition result: %i + %i = %i", v, adder, sum);

		// loop until server death notification
		if (v==0) {

//
// register for binder death notification
//
#if	!defined(LINK_TO_DEATH_IN_CLIENT)
			// No death notification with this approach
			sp<DeathNotifier> spDeathNotifier = new DeathNotifier();
			status_t rc = binder->linkToDeath(spDeathNotifier);
			INFO("linkToDeath: rc=%d (tid=%lx", rc, pthread_self());
#endif	//!LINK_TO_DEATH_IN_CLIENT

				// prepare a structure to hold android::sp
				thread_data threaddata;
				threaddata.spIDemo = demo;
				threaddata.spIBinder = binder;

#if	defined(START_CLIENT_THREAD)
			// start client thread
			pthread_t tid;
			pthread_create(&tid, NULL, clnt, &threaddata);
			printf("client thread id:%lx...\n",tid);
			printf("-->>> %s: thread id:%lx...\n",__func__,pthread_self());

			//https://groups.google.com/forum/#!topic/android-platform/vt3xsM2kcTM
			// we need a thread pool to receive binder callbacks (e.g., for DeathRecipient)
			sp<ProcessState> proc(ProcessState::self());
			INFO("starting threaed pool...");
			// startThreadPool() required for main() to receive death notification
			ProcessState::self()->startThreadPool();
///			IPCThreadState::self()->joinThreadPool();
			// pthread_join() required as clnt() thread was launched via pthread_create()
			pthread_join(tid, NULL);

		//
		// if we ever get out of joinThreadPool() via IPCThreadState::self()->stopProcess();
		//
///			android::IPCThreadState::shutdown();
#else
			sp<ProcessState> proc(ProcessState::self());
			INFO("starting threaed pool...");
			ProcessState::self()->startThreadPool();

			clnt(&threaddata);

			// no need for joinThreadPool() nor shutdown()
			//IPCThreadState::self()->joinThreadPool();
			//android::IPCThreadState::shutdown();
#endif	//!START_CLIENT_THREAD

			ALOGD("main exiting... gClientRun=%d",gClientRun);
		}

    }

    return 0;
}

/*
    Single-threaded service, single-threaded client.
 */
