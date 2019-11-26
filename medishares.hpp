#include <eosiolib/eosio.hpp>
#include <eosiolib/transaction.hpp>
#include <eosiolib/asset.hpp>
#include <string>

using namespace eosio;
using std::string;
using namespace std;

class hbtcoop: public eosio::contract{
  public:
    hbtcoop(account_name self):
    contract(self),
	global(_self, _self){}

    ///@abi action
	void add(const account_name initiator, const std::string name, const checksum256 item_digest, const account_name receiver, const extended_asset& min_fund, const extended_asset& max_fund, const extended_asset& target_fund, const uint64_t deadline);

    ///@abi action
	void modify(const account_name initiator, const uint64_t id, const std::string name, const checksum256 item_digest, const account_name receiver, const extended_asset& min_fund, const extended_asset& max_fund, const extended_asset& target_fund, const uint64_t deadline);

    ///@abi action
	void erase(const account_name initiator, const std::string name);
	
	///@abi action
	void audit(const account_name initiator, const std::string name, const uint8_t flag);
	
	///@abi action
    void setauditor(const account_name auditor);
	
  private:
	///@abi table
	struct global
	{
		uint64_t total_item;
		uint64_t completed_item;
		uint64_t ongoing_item;
		account_name auditor;//项目审核员，可以为多签账户
	
		auto primary_key()const{return 0;}
		EOSLIB_SERIALIZE(global, (total_item)(completed_item)(ongoing_item)(auditor))
	};
  	eosio::multi_index<N(global), global> global;

	///@abi table
    struct item
    {
		uint64_t		id=1;
		std::string 	name;
		checksum256		item_digest;
		account_name	initiator;
		account_name	receiver;
        extended_asset  min_fund;
        extended_asset  max_fund;
		extended_asset  target_fund;
		time			start;
		time 			deadline;
		uint8_t			status; //0：待审核，1：待修改，2：互助中，3：赔付结束
		
        auto primary_key()const{return id;}
        EOSLIB_SERIALIZE(item, (id)(name)(item_digest)(initiator)(receiver)(min_fund)(max_fund)(target_fund)(start)(deadline)(status))
    };
    typedef eosio::multi_index<N(item), item> items_table;

};

EOSIO_ABI(hbtcoop, (add)(modify)(erase)(audit)(setauditor))

