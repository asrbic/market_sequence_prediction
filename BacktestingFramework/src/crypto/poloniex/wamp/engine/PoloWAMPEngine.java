package crypto.poloniex.wamp.engine;

import java.util.concurrent.TimeUnit;

import rx.Scheduler;
import rx.Subscription;
import rx.functions.Action1;
import rx.schedulers.Schedulers;
import ws.wamp.jawampa.PubSubData;
import ws.wamp.jawampa.WampClient;
import ws.wamp.jawampa.WampClient.State;
import ws.wamp.jawampa.WampClientBuilder;
import ws.wamp.jawampa.connection.IWampConnectorProvider;
import ws.wamp.jawampa.transport.netty.NettyWampClientConnectorProvider;

public class PoloWAMPEngine {
	WampClient client;
	Action1 controllerSubscriber;
	Subscription tickerSub;
	Scheduler sched = Schedulers.io();
	public PoloWAMPEngine(Action1 controllerSubscriber) {
		client = null;
		this.controllerSubscriber = controllerSubscriber;
	}
	
	public void doConnect() {
		if(client != null) {
			close();
		}
		try {
			IWampConnectorProvider cp = new NettyWampClientConnectorProvider();
		    // Create a builder and configure the client
		    WampClientBuilder builder = new WampClientBuilder();
		    builder.withConnectorProvider(cp)
		           .withUri("wss://api.poloniex.com")
		           .withRealm("realm1")
		           .withInfiniteReconnects()
		           .withReconnectInterval(5, TimeUnit.SECONDS);
		    // Create a client through the builder. This will not immediatly start
		    // a connection attempt
		    client = builder.build();
		    client.statusChanged()
		      .observeOn(sched)
		      .subscribe(controllerSubscriber);
		    
		    client.open();
		} catch (Exception e) {
		    // Catch exceptions that will be thrown in case of invalid configuration
		    System.err.println(e);
		    if(client != null) {
		    	client.close();
		    }
		    throw new RuntimeException(e);
		}
		client.toString();
	}
	
	public void startTickerSub() {
	    tickerSub = client.makeSubscription("ticker")
    	.observeOn(sched)
	    .subscribe(controllerSubscriber);
	}
	
	public void close() {
		if(client != null) {
			client.close();
			client = null;
		}
	}

}
