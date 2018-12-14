#include <eosiolib/eosio.hpp>
#include <eosiolib/currency.hpp>

using namespace eosio;

class hello : public eosio::contract {
  public:
      using contract::contract;

      hello(account_name self) : contract(self) {}

      /// @abi table 
      struct account {
      	account_name owner;
      	uint64_t balance;

      	uint64_t primary_key() const { return owner; };
      };

      typedef eosio::multi_index<N(account), account> accs; 

      /// @abi action 
      void addbalance(account_name owner, uint64_t amount) {
      	 accs accounts(_self, owner);
      	 auto existing = accounts.find(owner);

      	 if (existing == accounts.end())
      	 {
      	 	/* Add new account */
      	 	accounts.emplace(_self, [&](auto& a) {
      	 		a.owner = owner;
      	 		a.balance = amount;
      	 	});
      	 } else {
      	 	/* Update existing account */
      	 	const auto& st = *existing;
      	 	accounts.modify(st, 0, [&](auto& a) {
      	 		a.balance += amount;
      	 	});
      	 }
      }

      void on(const currency::transfer& t) {
      	if (t.to == _self) {
      		addbalance(t.from, t.quantity.amount);
      	}
      }

      void use(account_name contract, account_name act) {
      	if (act == N(transfer)) {
      		on(unpack_action_data<currency::transfer>());
      		return;
      	}

      	auto& thiscontract = *this;
      	switch( act ) {
         	EOSIO_API( hello, (addbalance));
      	};
      }
};

extern "C" {
   [[noreturn]] void apply( uint64_t receiver, uint64_t code, uint64_t action ) {
      hello  h( receiver );
      h.use( code, action );
      eosio_exit(0);
   }
}